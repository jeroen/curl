/* Streaming interface to libcurl for R. (c) 2014 Jeroen Ooms.
 * Source: https://github.com/jeroenooms/curl
 * Curl example: example: http://curl.haxx.se/libcurl/c/getinmemory.html
 * Rconnection interface: http://biostatmatt.com/R/R-conn-ints/C-Structures.html
 */

#include <curl/curl.h>
#include <curl/easy.h>
#include <Rinternals.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <R_ext/Connections.h>
#if ! defined(R_CONNECTIONS_VERSION) || R_CONNECTIONS_VERSION != 1
#error "Unsupported connections API version"
#endif

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define R_EOF -1

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
  int has_more;
} curl_private;

/* example: http://curl.haxx.se/libcurl/c/getinmemory.html */
static size_t push(void *contents, size_t size, size_t nmemb, curl_private *cc) {
  size_t newsize = size * nmemb;
  if((cc->size + newsize) > (cc->limit))
    error("Buffer overflow in push!");
  memcpy(&(cc->buf[cc->size]), contents, newsize);
  cc->size = cc->size + newsize;
  Rprintf("Pushed %d bytes. New size:%d bytes.\n", newsize, cc->size);
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
  Rprintf("Requested %d bytes, popped %d bytes, new size %d bytes.\n", req_size, copy_size, cc->size);
  return copy_size;
}

void assert(CURLMcode res){
  if(res != CURLE_OK)
    error(curl_easy_strerror(res));
}

SEXP R_curl_connection(SEXP url) {
  if(!isString(url))
    error("Argument 'url' must be string.");

  /* create the R connection object */
  Rconnection con;
  SEXP rc = R_new_custom_connection(translateCharUTF8(asChar(url)), "", "curl", &con);

  /* create the internal curl structure */
  curl_private *cc;
  cc = malloc(sizeof(curl_private));
  cc->url = translateCharUTF8(asChar(url));
  cc->limit = 2 * CURL_MAX_WRITE_SIZE;

  /* set connection properties */
  con->private = cc;
  con->canseek = FALSE;
  con->canwrite = FALSE;
  con->isopen = FALSE;
  con->blocking = TRUE;
  con->text = FALSE;
  con->UTF8out = TRUE;
  con->open = rcurl_open;
  con->destroy = cleanup;
  con->read = rcurl_read;
  con->fgetc = rcurl_fgetc;
  return rc;
}

static Rboolean rcurl_open(Rconnection con) {
  CURL *http_handle;
  CURLM *multi_handle;

  /* get url value */
  curl_private *cc = (curl_private*) con->private;
  Rprintf("Opening URL:%s\n", cc->url);

  /* init */
  curl_global_init(CURL_GLOBAL_DEFAULT);

  /* setup http handler */
  http_handle = curl_easy_init();
  curl_easy_setopt(http_handle, CURLOPT_URL, cc->url);
  curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYPEER, 0L);

  /* set http request headers */
  struct curl_slist *reqheaders = NULL;
  reqheaders = curl_slist_append(reqheaders, "User-Agent: curl from r");
  reqheaders = curl_slist_append(reqheaders, "Accept-Charset: utf-8");
  reqheaders = curl_slist_append(reqheaders, "Cache-Control: no-cache");
  curl_easy_setopt(http_handle, CURLOPT_HTTPHEADER, reqheaders);

  /* init a multi stack */
  multi_handle = curl_multi_init();
  curl_multi_add_handle(multi_handle, http_handle);

  /* setup data callback function */
  curl_easy_setopt(http_handle, CURLOPT_WRITEFUNCTION, push);
  curl_easy_setopt(http_handle, CURLOPT_WRITEDATA, cc);

  /* we start some action by calling perform right away */
  assert(curl_multi_perform(multi_handle, &(cc->has_more)));

  /* store in struct */
  cc->buf = malloc(cc->limit);
  cc->size = 0;
  cc->http_handle = http_handle;
  cc->multi_handle = multi_handle;

  /* return the R connection object */
  con->isopen = TRUE;
  return TRUE;
}

/* Support for readBin() */
static size_t rcurl_read(void *buf, size_t sz, size_t ni, Rconnection con) {
  curl_private *cc = (curl_private*) con->private;
  size_t req_size = sz * ni;
  long timeout = 10*1000;

  /* clear existing buffer */
  size_t total_size = pop(buf, req_size, cc);

  /* fetch more data */
  while((req_size > total_size) && cc->has_more) {
    assert(curl_multi_timeout(cc->multi_handle, &timeout));
    assert(curl_multi_perform(cc->multi_handle, &(cc->has_more)));
    total_size = total_size + pop(&(buf[total_size]), (req_size-total_size), cc);
    //sleep(1L);
  }

  return total_size;
}

/* placeholder for readLines */
static int rcurl_fgetc(Rconnection c) {
  //return R_EOF;
  int r = rand() % 26;
  if(rand()%1000) {
    return r + 97;
  } else {
    return R_EOF;
  }
}

void cleanup(Rconnection con) {
  Rprintf("Cleaning up curl.\n");
  curl_private *cc = (curl_private*) con->private;
  curl_multi_remove_handle(cc->multi_handle, cc->http_handle);
  curl_easy_cleanup(cc->http_handle);
  curl_multi_cleanup(cc->multi_handle);
  /* NOTE: global_cleanup destroys all other connections as well */
  //curl_global_cleanup();
}
