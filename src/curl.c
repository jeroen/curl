/* *
 * Streaming interface to libcurl for R. (c) 2014 Jeroen Ooms.
 * Source: https://github.com/jeroenooms/curl
 * Comments and contributions are welcome!
 * Helpful libcurl examples:
 *  - http://curl.haxx.se/libcurl/c/getinmemory.html
 *  - http://curl.haxx.se/libcurl/c/multi-single.html
 * Info about Rconnection API:
 *  - https://github.com/wch/r-source/blob/trunk/src/include/R_ext/Connections.h
 *  - http://biostatmatt.com/R/R-conn-ints/C-Structures.html
 */
#include <curl/curl.h>
#include <curl/easy.h>
#include <Rinternals.h>
#include <string.h>
#include <stdlib.h>

#include <R_ext/Connections.h>
#if ! defined(R_CONNECTIONS_VERSION) || R_CONNECTIONS_VERSION != 1
#error "Unsupported connections API version"
#endif

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define R_EOF -1
#define BUFFER_LIMIT (2 * CURL_MAX_WRITE_SIZE)

static Rboolean rcurl_open(Rconnection c);
static size_t rcurl_read(void *buf, size_t sz, size_t ni, Rconnection c);
static int rcurl_fgetc(Rconnection c);
void cleanup(Rconnection con);

typedef struct {
  const char *url;
  CURL *http_handle;
  CURLM *multi_handle;
  char *buf;
  size_t size;
  size_t limit;
  int has_data;
  int has_more;
} curl_private;

/* example: http://curl.haxx.se/libcurl/c/getinmemory.html */
static size_t push(void *contents, size_t size, size_t nmemb, curl_private *cc) {
  cc->has_data = 1;
  size_t newsize = size * nmemb;
  if((cc->size + newsize) > (cc->limit))
    error("Buffer overflow in push!");
  memcpy(&(cc->buf[cc->size]), contents, newsize);
  cc->size = cc->size + newsize;
  //Rprintf("Pushed %d bytes. New size:%d bytes.\n", newsize, cc->size);
  return cc->size;
}

static size_t pop(void *target, size_t req_size, curl_private *cc){
  size_t copy_size = min(cc->size, req_size);
  if(copy_size){
    memcpy(target, cc->buf, copy_size);
    cc->size = cc->size - copy_size;
    if(cc->size > 0)
      memcpy(cc->buf, &(cc->buf[req_size]), cc->size);
  }
  //Rprintf("Requested %d bytes, popped %d bytes, new size %d bytes.\n", req_size, copy_size, cc->size);
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

  /* create the internal curl structure */
  curl_private *cc;
  cc = malloc(sizeof(curl_private));
  cc->url = translateCharUTF8(asChar(url));
  cc->limit = BUFFER_LIMIT;
  cc->http_handle = curl_easy_init();
  cc->multi_handle = curl_multi_init();
  curl_multi_add_handle(cc->multi_handle, cc->http_handle);

  /* set connection properties */
  con->private = cc;
  con->canseek = FALSE;
  con->canwrite = FALSE;
  con->isopen = FALSE;
  con->blocking = TRUE;
  con->text = TRUE;
  con->UTF8out = TRUE;
  con->open = rcurl_open;
  con->destroy = cleanup;
  con->read = rcurl_read;
  con->fgetc = rcurl_fgetc;

  /* open connection  */
  const char *smode = translateCharUTF8(asChar(mode));
  if(!strcmp(smode, "r") || !strcmp(smode, "rb")){
    rcurl_open(con);
  } else if(strcmp(smode, "")) {
    error("Invalid mode: %s", smode);
  }
  return rc;
}

static Rboolean rcurl_open(Rconnection con) {
  /* get url value */
  curl_private *cc = (curl_private*) con->private;
  CURL *http_handle = cc->http_handle;
  CURLM *multi_handle = cc->multi_handle;

  //Rprintf("Opening URL:%s\n", cc->url);

  /* setup http handler */
  curl_easy_setopt(http_handle, CURLOPT_URL, cc->url);
  curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(http_handle, CURLOPT_FOLLOWLOCATION, 1L);

  /* set http request headers */
  struct curl_slist *reqheaders = NULL;
  reqheaders = curl_slist_append(reqheaders, "User-Agent: r/curl/jeroen");
  reqheaders = curl_slist_append(reqheaders, "Accept-Charset: utf-8");
  reqheaders = curl_slist_append(reqheaders, "Cache-Control: no-cache");
  curl_easy_setopt(http_handle, CURLOPT_HTTPHEADER, reqheaders);

  /* init a multi stack with callback */
  curl_easy_setopt(http_handle, CURLOPT_WRITEFUNCTION, push);
  curl_easy_setopt(http_handle, CURLOPT_WRITEDATA, cc);

  /* store in private struct */
  cc->buf = malloc(cc->limit);
  cc->size = 0;
  cc->has_data = 0;
  cc->has_more = 1;

  /* Wait for first data to come in. Monitoring a change in status code does not
     suffice in case of http redirects */
  while(cc->has_more && !cc->has_data) {
    massert(curl_multi_perform(multi_handle, &(cc->has_more)));
    check_status(multi_handle);
  }

  long status = 0;
  assert(curl_easy_getinfo(http_handle, CURLINFO_RESPONSE_CODE, &status));

  /* check http status code. Not sure what this does for ftp. */
  if(status >= 300)
    error("HTTP error %d.", status);

  /* return the R connection object */
  con->isopen = TRUE;
  con->text = FALSE;
  strcpy(con->mode, "rb");
  return TRUE;
}

/* Support for readBin() */
static size_t rcurl_read(void *buf, size_t sz, size_t ni, Rconnection con) {
  buf = (int*) buf;
  curl_private *cc = (curl_private*) con->private;
  size_t req_size = sz * ni;
  long timeout = 10*1000;

  /* clear existing buffer */
  size_t total_size = pop(buf, req_size, cc);

  /* fetch more data */
  while((req_size > total_size) && cc->has_more) {
    massert(curl_multi_timeout(cc->multi_handle, &timeout));
    massert(curl_multi_perform(cc->multi_handle, &(cc->has_more)));
    check_status(cc->multi_handle);
    total_size = total_size + pop(&(buf[total_size]), (req_size-total_size), cc);
  }
  return total_size;
}

/* naive (slow) implementation of readLines */
static int rcurl_fgetc(Rconnection con) {
  int x;
  return rcurl_read(&x, 1, 1, con) ? x : R_EOF;
}

void cleanup(Rconnection con) {
  //Rprintf("Cleaning up curl.\n");
  curl_private *cc = (curl_private*) con->private;
  curl_multi_remove_handle(cc->multi_handle, cc->http_handle);
  curl_easy_cleanup(cc->http_handle);
  curl_multi_cleanup(cc->multi_handle);
  /* NOTE: global_cleanup destroys all other connections as well */
  //curl_global_cleanup();
}
