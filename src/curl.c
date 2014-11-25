/* *
 * Streaming interface to libcurl for R. (c) 2014 Jeroen Ooms.
 * Source: https://github.com/jeroenooms/curl
 * Comments and contributions are welcome!
 * Helpful libcurl examples:
 *  - http://curl.haxx.se/libcurl/c/getinmemory.html
 *  - http://curl.haxx.se/libcurl/c/multi-single.html
 * Sparse documentation about Rconnection API:
 *  - https://github.com/wch/r-source/blob/trunk/src/include/R_ext/Connections.h
 *  - http://biostatmatt.com/R/R-conn-ints/C-Structures.html
 *
 * Notes: the close() function in R actually calls con->destroy. The con->close
 * function is only used when a connection is recycled after auto-open.
 */
#include <curl/curl.h>
#include <curl/easy.h>
#include <Rinternals.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

/* the RConnection API is experimental and subject to change */
#include <R_ext/Connections.h>
#if ! defined(R_CONNECTIONS_VERSION) || R_CONNECTIONS_VERSION != 1
#error "Unsupported connections API version"
#endif

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define R_EOF -1

typedef struct {
  char *url;
  char *buf;
  char *cur;
  int has_data;
  int has_more;
  int used;
  size_t size;
  size_t limit;
  CURLM *manager;
  CURL *handle;
} request;

/* callback function to store received data */
static size_t push(void *contents, size_t sz, size_t nmemb, void *ctx) {
  /* avoids compiler warning on windows */
  request* req = (request*) ctx;
  req->has_data = 1;

  /* move existing data to front of buffer (if any) */
  memcpy(req->buf, req->cur, req->size);

  /* allocate more space if required */
  size_t realsize = sz * nmemb;
  size_t newsize = req->size + realsize;
  if(newsize > req->limit) {
    size_t newlimit = 2 * req->limit;
    //Rprintf("Resizing buffer to %d.\n", newlimit);
    void *newbuf = realloc(req->buf, newlimit);
    if(!newbuf)
      error("Failure in realloc. Out of memory?");
    req->buf = newbuf;
    req->limit = newlimit;
  }

  /* append new data */
  memcpy(req->buf + req->size, contents, realsize);
  req->size = newsize;
  req->cur = req->buf;
  return realsize;
}

static size_t pop(void *target, size_t max, request *req){
  size_t copy_size = min(req->size, max);
  memcpy(target, req->cur, copy_size);
  req->cur = req->cur + copy_size;
  req->size = req->size - copy_size;
  //Rprintf("Requested %d bytes, popped %d bytes, new size %d bytes.\n", max, copy_size, req->size);
  return copy_size;
}

void massert(CURLMcode res){
  if(res != CURLM_OK)
    error(curl_multi_strerror(res));
}

void check_manager(CURLM *manager) {
  for(int msg = 1; msg > 0;){
    CURLMsg *out = curl_multi_info_read(manager, &msg);
    if(out)
      assert(out->data.result);
  }
}

void fetch(request *req) {
  R_CheckUserInterrupt();
  long timeout = 10*1000;
  massert(curl_multi_timeout(req->manager, &timeout));
  massert(curl_multi_perform(req->manager, &(req->has_more)));
  check_manager(req->manager);
}

/* Support for readBin() */
static size_t rcurl_read(void *target, size_t sz, size_t ni, Rconnection con) {
  request *req = (request*) con->private;
  size_t req_size = sz * ni;

  /* append data to the target buffer */
  size_t total_size = pop(target, req_size, req);
  while((req_size > total_size) && req->has_more) {
    fetch(req);
    total_size += pop((char*)target + total_size, (req_size-total_size), req);
  }
  return total_size;
}

/* naive implementation of readLines */
static int rcurl_fgetc(Rconnection con) {
  int x = 0;
  return rcurl_read(&x, 1, 1, con) ? x : R_EOF;
}

void cleanup(Rconnection con) {
  //Rprintf("Destroying connection.\n");
  request *req = (request*) con->private;
  curl_multi_remove_handle(req->manager, req->handle);
  curl_easy_cleanup(req->handle);
  curl_multi_cleanup(req->manager);
  free(req->buf);
  free(req->url);
  free(req);
}

/* reset to pre-opened state */
void reset(Rconnection con) {
  //Rprintf("Closing connection.\n");
  con->isopen = FALSE;
  con->text = TRUE;
  strcpy(con->mode, "r");
}

static Rboolean rcurl_open(Rconnection con) {
  request *req = (request*) con->private;
  //Rprintf("Opening URL:%s\n", req->url);

  /* case of recycled connection */
  if(req->used) {
    //Rprintf("Cleaning up old handle.\n");
    curl_multi_remove_handle(req->manager, req->handle);
    curl_easy_cleanup(req->handle);
  }

  /* init a multi stack with callback */
  CURL *handle = make_handle(req->url);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, push);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, req);
  curl_multi_add_handle(req->manager, handle);

  /* reset the state */
  req->handle = handle;
  req->cur = req->buf;
  req->size = 0;
  req->used = 1;
  req->has_data = 0;
  req->has_more = 1;

  /* Wait for first data to arrive. Monitoring a change in status code does not
     suffice in case of http redirects */
  while(req->has_more && !req->has_data) {
    fetch(req);
  }

  /* check http status code */
  stop_for_status(handle);

  /* set mode in case open() changed it */
  con->text = strcmp(con->mode, "rb") ? TRUE : FALSE;
  con->isopen = TRUE;
  return TRUE;
}

SEXP R_curl_connection(SEXP url, SEXP mode) {
  if(!isString(url))
    error("Argument 'url' must be string.");

  if(!isString(mode))
    error("Argument 'mode' must be string.");

  /* maybe not be needed as curl_easy_init triggers a global_init as well  */
  curl_global_init(CURL_GLOBAL_DEFAULT);

  /* create the R connection object */
  Rconnection con;
  SEXP rc = PROTECT(R_new_custom_connection(translateCharUTF8(asChar(url)), "r", "curl", &con));

  /* setup curl. These are the parts that are recycable. */
  request *req = malloc(sizeof(request));
  req->limit = CURL_MAX_WRITE_SIZE;
  req->buf = malloc(req->limit);
  req->manager = curl_multi_init();
  req->used = 0;

  /* allocate url string */
  req->url = malloc(strlen(translateCharUTF8(asChar(url))+1));
  strcpy(req->url, translateCharUTF8(asChar(url)));

  /* set connection properties */
  con->private = req;
  con->canseek = FALSE;
  con->canwrite = FALSE;
  con->isopen = FALSE;
  con->blocking = TRUE;
  con->text = TRUE;
  con->UTF8out = TRUE;
  con->open = rcurl_open;
  con->close = reset;
  con->destroy = cleanup;
  con->read = rcurl_read;
  con->fgetc = rcurl_fgetc;
  con->fgetc_internal = rcurl_fgetc;

  /* open connection  */
  const char *smode = translateCharUTF8(asChar(mode));
  if(!strcmp(smode, "r") || !strcmp(smode, "rb")){
    strcpy(con->mode, smode);
    rcurl_open(con);
  } else if(strcmp(smode, "")) {
    error("Invalid mode: %s", smode);
  }
  UNPROTECT(1);
  return rc;
}
