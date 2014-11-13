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
static int wait_on_socket(curl_socket_t sockfd, int for_recv, long timeout_ms);

#include <R_ext/Connections.h>
#if ! defined(R_CONNECTIONS_VERSION) || R_CONNECTIONS_VERSION != 1
#error "Unsupported connections API version"
#endif


SEXP R_curl_connection(SEXP url) {
  if(!isString(url))
    error("Argument 'url' must be string.");

  Rconnection con;
  SEXP rc = R_new_custom_connection(CHAR(asChar(url)), "", "curl", &con);

  //cc->sub = 0;
  //cc->msg_ptr = -1;
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

  if (con->mode[0] && !con->open(con)) /* auto-open */
	  Rf_error("cannot open the connection");
  return rc;
}

static Rboolean rcurl_open(Rconnection con) {
  request(con);
  con->isopen = TRUE;
  return TRUE;
}

static void rcurl_close(Rconnection con) {
  //curl_easy_cleanup(con->private);
}

static size_t rcurl_read(void *buf, size_t sz, size_t ni, Rconnection con) {
  size_t req = sz * ni;
  const char test[] = "Foo Bar";
  memcpy(buf, (unsigned char*) &test, req);
  return strlen(test);
}

/* needed to support text mode */
static int rcurl_fgetc(Rconnection c) {
  //return R_EOF;
  int r = rand() % 26;
  if(rand()%1000){
    return r + 97;
  } else {
    return R_EOF;
  }
}

void request(Rconnection con) {
  Rprintf("Opening URL:%s\n", con->description);
  CURL *curl;
  CURLcode res;
  size_t iolen;
  curl_off_t nread;
  curl_socket_t sockfd;
  long sockextr;

  const char *request = "GET /get HTTP/1.0\r\nHost: httpbin.org\r\n\r\n";

  /* set request options */
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, con->description);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

  /* set http request headers */
  struct curl_slist *reqheaders = NULL;
  reqheaders = curl_slist_append(reqheaders, "User-Agent: curl from r");
  reqheaders = curl_slist_append(reqheaders, "Accept-Charset: utf-8");
  reqheaders = curl_slist_append(reqheaders, "Cache-Control: no-cache");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, reqheaders);

  /* set output target */
  //FILE *f = fopen("/dev/null", "wb");
  //curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);

  //curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
  curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);

  res = curl_easy_perform(curl);
  if(res != CURLE_OK)
    error(curl_easy_strerror(res));

  res = curl_easy_getinfo(curl, CURLINFO_LASTSOCKET, &sockextr);
  if(CURLE_OK != res)
    error(curl_easy_strerror(res));

  sockfd = sockextr;
  if(!wait_on_socket(sockfd, 0, 60000L))
    error("timeout.");

  res = curl_easy_send(curl, request, strlen(request), &iolen);
  if(CURLE_OK != res)
    error(curl_easy_strerror(res));

  for(;;) {
    char buf[1024];
    wait_on_socket(sockfd, 1, 60000L);
    res = curl_easy_recv(curl, buf, 1024, &iolen);
    if(CURLE_OK != res)
      break;

    nread = (curl_off_t)iolen;
    Rprintf("Received %" CURL_FORMAT_CURL_OFF_T " bytes.\n", nread);
  }

  con->private = curl;
}

/* Auxiliary function that waits on the socket. */
static int wait_on_socket(curl_socket_t sockfd, int for_recv, long timeout_ms) {
  struct timeval tv;
  fd_set infd, outfd, errfd;
  int res;

  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec= (timeout_ms % 1000) * 1000;

  FD_ZERO(&infd);
  FD_ZERO(&outfd);
  FD_ZERO(&errfd);

  FD_SET(sockfd, &errfd); /* always check for error */

  if(for_recv)
  {
    FD_SET(sockfd, &infd);
  }
  else
  {
    FD_SET(sockfd, &outfd);
  }

  /* select() returns the number of signalled sockets or -1 */
  res = select(sockfd + 1, &infd, &outfd, &errfd, &tv);
  return res;
}
