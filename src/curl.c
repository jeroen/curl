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
  CURL *http_handle;
  CURLM *multi_handle;
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
  CURL *http_handle;
  CURLM *multi_handle;
  CURLcode res;
  int still_running = 1;

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

  /* should plugin an R connection or fd here */
  //FILE *f = fopen("/dev/null", "wb");
  //curl_easy_setopt(http_handle, CURLOPT_WRITEDATA, f);

  /* we start some action by calling perform right away */
  res = curl_multi_perform(multi_handle, &still_running);
  if(res != CURLE_OK)
    error(curl_easy_strerror(res));

  /* this should move to readBin */
  while(still_running) {
    struct timeval timeout;

    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd = -1;

    long curl_timeo = -1;

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    /* set a suitable timeout to play around with */
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    curl_multi_timeout(multi_handle, &curl_timeo);
    if(curl_timeo >= 0) {
      timeout.tv_sec = curl_timeo / 1000;
      if(timeout.tv_sec > 1)
        timeout.tv_sec = 1;
      else
        timeout.tv_usec = (curl_timeo % 1000) * 1000;
    }

    /* get file descriptors from the transfers */
    res = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
    if(res != CURLE_OK)
      error(curl_easy_strerror(res));

    /* In a real-world program you OF COURSE check the return code of the
       function calls.  On success, the value of maxfd is guaranteed to be
       greater or equal than -1.  We call select(maxfd + 1, ...), specially in
       case of (maxfd == -1), we call select(0, ...), which is basically equal
       to sleep. */

    int rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

    switch(rc) {
    case -1:
      still_running = 0;
      error("select() returns error, this is badness.");
      break;
    case 0:
    default:
      /* timeout or readable/writable sockets */
      curl_multi_perform(multi_handle, &still_running);
      break;
    }
  }

  /* cleanup should be done in close() function */
  curl_multi_remove_handle(multi_handle, http_handle);
  curl_easy_cleanup(http_handle);
  curl_multi_cleanup(multi_handle);
  curl_global_cleanup();
}
