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
 * Notes: the close() function in R actually calls cc->destroy. The cc->close
 * function is only used when a connection is recycled.
 */
#include <curl/curl.h>
#include <curl/easy.h>
#include <Rinternals.h>
#include <string.h>
#include <stdlib.h>

/* the RConnection API is experimental and subject to change */
#include <R_ext/Connections.h>
#if ! defined(R_CONNECTIONS_VERSION) || R_CONNECTIONS_VERSION != 1
#error "Unsupported connections API version"
#endif

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define R_EOF -1

typedef struct {
  const char *url;
  CURL *http_handle;
  CURLM *multi_handle;
  char *buf;
  char *ptr;
  size_t size;
  size_t limit;
  int has_data;
  int has_more;
  int used;
} curl_private;

/* callback function to store received data */
static size_t push(void *contents, size_t sz, size_t nmemb, curl_private *cc) {
  /* only needed first time */
  cc->has_data = 1;
  size_t realsize = sz * nmemb;
  size_t newsize = cc->size + realsize;

  /* move existing data to front of buffer */
  memcpy(cc->buf, cc->ptr, cc->size);

  /* allocate more space if required */
  if(newsize > cc->limit) {
    //Rprintf("Resizing buffer to %d.\n", newsize);
    void *newbuf = realloc(cc->buf, newsize + 1);
    if(!newbuf)
      error("Failure in realloc. Out of memory?");
    cc->buf = newbuf;
    cc->limit = newsize;
  }

  /* append new data */
  memcpy(cc->buf + cc->size, contents, realsize);
  cc->size = newsize;
  cc->ptr = cc->buf;
  return realsize;
}

static size_t pop(void *target, size_t max, curl_private *cc){
  size_t copy_size = min(cc->size, max);
  memcpy(target, cc->ptr, copy_size);
  cc->ptr = cc->ptr + copy_size;
  cc->size = cc->size - copy_size;
  //Rprintf("Requested %d bytes, popped %d bytes, new size %d bytes.\n", max, copy_size, cc->size);
  return copy_size;
}

void assert(CURLcode res){
  if(res != CURLE_OK)
    error(curl_easy_strerror(res));
}

void massert(CURLMcode res){
  if(res != CURLM_OK)
    error(curl_multi_strerror(res));
}

void check_status(CURLM *multi_handle) {
  for(int msg = 1; msg > 0;){
    CURLMsg *out = curl_multi_info_read(multi_handle, &msg);
    if(out)
      assert(out->data.result);
  }
}

void fetch(curl_private *cc) {
  long timeout = 10*1000;
  massert(curl_multi_timeout(cc->multi_handle, &timeout));
  massert(curl_multi_perform(cc->multi_handle, &(cc->has_more)));
  check_status(cc->multi_handle);
}

/* Support for readBin() */
static size_t rcurl_read(void *target, size_t sz, size_t ni, Rconnection con) {
  curl_private *cc = (curl_private*) con->private;
  size_t req_size = sz * ni;

  /* append data to the target buffer */
  size_t total_size = pop(target, req_size, cc);
  while((req_size > total_size) && cc->has_more) {
    fetch(cc);
    total_size += pop((char *)target + total_size, (req_size-total_size), cc);
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
  curl_private *cc = (curl_private*) con->private;
  curl_multi_remove_handle(cc->multi_handle, cc->http_handle);
  curl_easy_cleanup(cc->http_handle);
  curl_multi_cleanup(cc->multi_handle);
  free(cc->buf);
  free(cc);
}

/* reset to pre-opened state */
void reset(Rconnection con) {
  //Rprintf("Closing connection.\n");
  con->isopen = FALSE;
  con->text = TRUE;
  strcpy(con->mode, "r");
}

static Rboolean rcurl_open(Rconnection con) {
  curl_private *cc = (curl_private*) con->private;
  //Rprintf("Opening URL:%s\n", cc->url);

  /* case of recycled connection */
  if(cc->used) {
    //Rprintf("Cleaning up old handle.\n");
    curl_multi_remove_handle(cc->multi_handle, cc->http_handle);
    curl_easy_cleanup(cc->http_handle);
  }

  CURL *http_handle = curl_easy_init();
  curl_multi_add_handle(cc->multi_handle, http_handle);

  /* reset the state */
  cc->http_handle = http_handle;
  cc->ptr = cc->buf;
  cc->size = 0;
  cc->used = 1;
  cc->has_data = 0;
  cc->has_more = 1;

  /* curl configuration options */
  curl_easy_setopt(http_handle, CURLOPT_URL, cc->url);
  curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(http_handle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(http_handle, CURLOPT_CONNECTTIMEOUT_MS, 10*1000);
  curl_easy_setopt(http_handle, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");

  /* set http request headers */
  struct curl_slist *reqheaders = NULL;
  reqheaders = curl_slist_append(reqheaders, "User-Agent: r/curl/jeroen");
  reqheaders = curl_slist_append(reqheaders, "Accept-Charset: utf-8");
  reqheaders = curl_slist_append(reqheaders, "Cache-Control: no-cache");
  curl_easy_setopt(http_handle, CURLOPT_HTTPHEADER, reqheaders);

  /* init a multi stack with callback */
  curl_easy_setopt(http_handle, CURLOPT_WRITEFUNCTION, push);
  curl_easy_setopt(http_handle, CURLOPT_WRITEDATA, cc);

  /* Wait for first data to arrive. Monitoring a change in status code does not
     suffice in case of http redirects */
  while(cc->has_more && !cc->has_data) {
    fetch(cc);
  }

  long status = 0;
  assert(curl_easy_getinfo(http_handle, CURLINFO_RESPONSE_CODE, &status));

  /* check http status code. Not sure what this does for ftp. */
  if(status >= 300)
    error("HTTP error %d.", status);

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
  SEXP rc = R_new_custom_connection(translateCharUTF8(asChar(url)), "r", "curl", &con);

  /* setup curl. These are the parts that are recycable. */
  curl_private *cc = malloc(sizeof(curl_private));
  cc->limit = CURL_MAX_WRITE_SIZE;
  cc->buf = malloc(cc->limit);
  cc->url = translateCharUTF8(asChar(url));
  cc->multi_handle = curl_multi_init();
  cc->used = 0;

  /* set connection properties */
  con->private = cc;
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

  /* open connection  */
  const char *smode = translateCharUTF8(asChar(mode));
  if(!strcmp(smode, "r") || !strcmp(smode, "rb")){
    strcpy(con->mode, smode);
    rcurl_open(con);
  } else if(strcmp(smode, "")) {
    error("Invalid mode: %s", smode);
  }
  return rc;
}

SEXP R_global_cleanup() {
  curl_global_cleanup();
  return R_NilValue;
}
