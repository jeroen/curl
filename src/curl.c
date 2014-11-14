#include <curl/curl.h>
#include <curl/easy.h>
#include <Rinternals.h>
#include <R_ext/Connections.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>

#define R_EOF -1

static Rboolean rcurl_open(Rconnection c);
static void rcurl_close(Rconnection c);
static size_t rcurl_read(void *buf, size_t sz, size_t ni, Rconnection c);
static int rcurl_fgetc(Rconnection c);
void request(Rconnection con);

#include <R_ext/Connections.h>
#if ! defined(R_CONNECTIONS_VERSION) || R_CONNECTIONS_VERSION != 1
#error "Unsupported connections API version"
#endif

typedef struct curl_private {
  const char *url;
  CURL *curl;
} curl_private;

SEXP R_curl_connection(SEXP url) {
  if(!isString(url))
    error("Argument 'url' must be string.");

  Rconnection con;
  SEXP rc = R_new_custom_connection(CHAR(asChar(url)), "", "curl", &con);

  curl_private *cc;
  cc = malloc(sizeof(curl_private));
  if (!cc) Rf_error("cannot allocate private context");

  cc->url = CHAR(asChar(url));
  con->private = cc;
  con->canseek = FALSE;
  con->canwrite = FALSE;
  con->isopen = FALSE;
  con->blocking = TRUE;
  con->text = FALSE;
  con->open = rcurl_open;
  con->close = rcurl_close;
  con->read = rcurl_read;
  con->fgetc = rcurl_fgetc;
  //con->write = zmqc_write;
  return rc;
}

static Rboolean rcurl_open(Rconnection con) {
  request(con);
  con->isopen = TRUE;
  return TRUE;
}

/* this doesn't work. Seems like con has been destroyed already */
static void rcurl_close(Rconnection con) {

}

/* placeholder for readBin */
static size_t rcurl_read(void *buf, size_t sz, size_t ni, Rconnection con) {
  size_t req = sz * ni;
  const char test[] = "Foo Bar";
  memcpy(buf, (unsigned char*) &test, req);
  return strlen(test);
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

void request(Rconnection con) {
  CURL *curl;
  CURLcode res;

  /* get url value */
  curl_private *cc = (curl_private*) con->private;
  Rprintf("Opening URL:%s\n", cc->url);

  /* set request options */
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, cc->url);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

  /* set http request headers */
  struct curl_slist *reqheaders = NULL;
  reqheaders = curl_slist_append(reqheaders, "User-Agent: curl from r");
  reqheaders = curl_slist_append(reqheaders, "Accept-Charset: utf-8");
  reqheaders = curl_slist_append(reqheaders, "Cache-Control: no-cache");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, reqheaders);

  /* should plugin an R connection or fd here */
  //FILE *f = fopen("/dev/null", "wb");
  //curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);

  res = curl_easy_perform(curl);
  if(res != CURLE_OK)
    error(curl_easy_strerror(res));

  curl_easy_cleanup(curl);
}
