/* *
 * Streaming interface to libcurl for R. (c) 2015 Jeroen Ooms.
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
#include "curl-common.h"
#include <Rconfig.h>

/* Define BSWAP_32 on Big Endian systems */
#if (defined(__sun) && defined(__SVR4))
#include <sys/byteorder.h>
#elif (defined(__APPLE__) && defined(__ppc__) || defined(__ppc64__))
#include <libkern/OSByteOrder.h>
#define BSWAP_32 OSSwapInt32
#elif (defined(__OpenBSD__))
#define BSWAP_32(x) swap32(x)
#endif

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
  int stop;
  size_t size;
  size_t limit;
  CURLM *manager;
  CURL *handle;
  reference *ref;
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
  req->cur += copy_size;
  req->size -= copy_size;
  //Rprintf("Requested %d bytes, popped %d bytes, new size %d bytes.\n", max, copy_size, req->size);
  return copy_size;
}

void check_manager(CURLM *manager) {
  for(int msg = 1; msg > 0;){
    CURLMsg *out = curl_multi_info_read(manager, &msg);
    if(out)
      assert(out->data.result);
  }
}

//NOTE: renamed because the name 'fetch' caused crash/conflict on Solaris.
void fetchdata(request *req) {
  R_CheckUserInterrupt();
  long timeout = 10*1000;
  massert(curl_multi_timeout(req->manager, &timeout));
  /* massert(curl_multi_perform(req->manager, &(req->has_more))); */

  /* On libcurl < 7.20 we need to check for CURLM_CALL_MULTI_PERFORM, see docs */
  CURLMcode res = CURLM_CALL_MULTI_PERFORM;
  while(res == CURLM_CALL_MULTI_PERFORM){
    res = curl_multi_perform(req->manager, &(req->has_more));
  }
  massert(res);
  /* End */
  check_manager(req->manager);
}

/* Support for readBin() */
static size_t rcurl_read(void *target, size_t sz, size_t ni, Rconnection con) {
  request *req = (request*) con->private;
  size_t req_size = sz * ni;

  /* append data to the target buffer */
  size_t total_size = pop(target, req_size, req);
  while((req_size > total_size) && req->has_more) {
    fetchdata(req);
    total_size += pop((char*)target + total_size, (req_size-total_size), req);
  }
  return total_size;
}

/* naive implementation of readLines */
static int rcurl_fgetc(Rconnection con) {
  int x = 0;
#ifdef WORDS_BIGENDIAN
  return rcurl_read(&x, 1, 1, con) ? BSWAP_32(x) : R_EOF;
#else
  return rcurl_read(&x, 1, 1, con) ? x : R_EOF;
#endif
}

void cleanup(Rconnection con) {
  //Rprintf("Destroying connection.\n");
  request *req = (request*) con->private;
  reference *ref = req->ref;

  /* free thee handle connection */
  curl_multi_remove_handle(req->manager, req->handle);
  ref->locked = 0;

  /* delayed finalizer cleanup */
  (ref->refCount)--;
  clean_handle(ref);

  /* clean up connection */
  curl_multi_cleanup(req->manager);
  free(req->buf);
  free(req->url);
  free(req);
}

/* reset to pre-opened state */
void reset(Rconnection con) {
  //Rprintf("Resetting connection object.\n");
  request *req = (request*) con->private;
  curl_multi_remove_handle(req->manager, req->handle);
  req->ref->locked = 0;
  con->isopen = FALSE;
  con->text = TRUE;
  strcpy(con->mode, "r");
}

static Rboolean rcurl_open(Rconnection con) {
  request *req = (request*) con->private;
  //Rprintf("Opening URL:%s\n", req->url);

  if(req->ref->locked)
    Rf_error("Handle is already in use elsewhere.");

  /* init a multi stack with callback */
  CURL *handle = req->handle;
  assert(curl_easy_setopt(handle, CURLOPT_URL, req->url));
  assert(curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, push));
  assert(curl_easy_setopt(handle, CURLOPT_WRITEDATA, req));

  /* add the handle to the pool and lock it */
  massert(curl_multi_add_handle(req->manager, handle));
  req->ref->locked = 1;

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
    fetchdata(req);
  }

  /* check http status code */
  if(req->stop)
    stop_for_status(handle);

  /* set mode in case open() changed it */
  con->text = strcmp(con->mode, "rb") ? TRUE : FALSE;
  con->isopen = TRUE;
  return TRUE;
}

SEXP R_curl_connection(SEXP url, SEXP mode, SEXP ptr, SEXP stop_on_error) {
  if(!isString(url))
    error("Argument 'url' must be string.");

  if(!isString(mode))
    error("Argument 'mode' must be string.");

  /* create the R connection object */
  Rconnection con;
  SEXP rc = PROTECT(R_new_custom_connection(translateCharUTF8(asChar(url)), "r", "curl", &con));

  /* setup curl. These are the parts that are recycable. */
  request *req = malloc(sizeof(request));
  req->handle = get_handle(ptr);
  req->ref = get_ref(ptr);
  req->limit = CURL_MAX_WRITE_SIZE;
  req->buf = malloc(req->limit);
  req->manager = curl_multi_init();
  req->used = 0;
  req->stop = asLogical(stop_on_error);

  /* allocate url string */
  req->url = malloc(strlen(translateCharUTF8(asChar(url))) + 1);
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
  const char *smode = CHAR(asChar(mode));
  if(!strcmp(smode, "r") || !strcmp(smode, "rb")){
    strcpy(con->mode, smode);
    rcurl_open(con);
  } else if(strcmp(smode, "")) {
    error("Invalid mode: %s", smode);
  }

  /* protect the handle */
  (req->ref->refCount)++;

  UNPROTECT(1);
  return rc;
}
