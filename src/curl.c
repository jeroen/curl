/* *
 * Streaming interface to libcurl for R. (c) 2015 Jeroen Ooms.
 * Source: https://github.com/jeroen/curl
 * Comments and contributions are welcome!
 * Helpful libcurl examples:
 *  - https://curl.se/libcurl/c/getinmemory.html
 *  - https://curl.se/libcurl/c/multi-single.html
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
#ifdef WORDS_BIGENDIAN
#if (defined(__sun) && defined(__SVR4))
#include <sys/byteorder.h>
#elif (defined(__APPLE__) && defined(__ppc__) || defined(__ppc64__))
#include <libkern/OSByteOrder.h>
#define BSWAP_32 OSSwapInt32
#elif (defined(__OpenBSD__))
#define BSWAP_32(x) swap32(x)
#elif (defined(__NetBSD__))
#include <sys/types.h>
#include <machine/bswap.h>
#define BSWAP_32(x) bswap32(x)
#elif (defined(__GLIBC__))
#include <byteswap.h>
#define BSWAP_32(x) bswap_32(x)
#elif (defined(_AIX))
#define BSWAP_32(x) __builtin_bswap32(x)
#endif
#endif

/* the RConnection API is experimental and subject to change */
#include <R_ext/Connections.h>
#if ! defined(R_CONNECTIONS_VERSION) || R_CONNECTIONS_VERSION != 1
#error "Unsupported connections API version"
#endif

/* for select() in connect-only (socket) mode */
#ifndef _WIN32
#include <sys/select.h>
#include <sys/time.h>
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
  int partial;
  int connect_only;
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
  memmove(req->buf, req->cur, req->size);

  /* allocate more space if required */
  size_t realsize = sz * nmemb;
  size_t newsize = req->size + realsize;
  while(newsize > req->limit) {
    size_t newlimit = 2 * req->limit;
    //Rprintf("Resizing buffer to %d.\n", newlimit);
    void *newbuf = realloc(req->buf, newlimit);
    if(!newbuf)
      Rf_error("Failure in realloc. Out of memory?");
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

static void check_handles(CURLM *manager, reference *ref) {
  for(int msg = 1; msg > 0;){
    CURLMsg *out = curl_multi_info_read(manager, &msg);
    if(out)
      assert_status(out->data.result, ref);
  }
}

static void fetchdata(request *req) {
  R_CheckUserInterrupt();
  massert(curl_multi_perform(req->manager, &(req->has_more)));
  check_handles(req->manager, req->ref);
}

/* wait for the socket of a connect-only handle to become readable/writable */
static int wait_on_socket(curl_socket_t sockfd, int for_recv, int timeout_ms) {
  struct timeval tv;
  fd_set infd, outfd, errfd;
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;
  FD_ZERO(&infd);
  FD_ZERO(&outfd);
  FD_ZERO(&errfd);
  FD_SET(sockfd, &errfd);
  FD_SET(sockfd, for_recv ? &infd : &outfd);
  return select((int) sockfd + 1, &infd, &outfd, &errfd, &tv);
}

static curl_socket_t get_active_socket(request *req){
  curl_socket_t sockfd = CURL_SOCKET_BAD;
  assert(curl_easy_getinfo(req->handle, CURLINFO_ACTIVESOCKET, &sockfd));
  if(sockfd == CURL_SOCKET_BAD)
    Rf_error("Connection has no active socket (already closed by peer?)");
  return sockfd;
}

/* read for connect-only connections: bytes come straight off the socket
 * via curl_easy_recv() instead of the multi write callback */
/* blocking reads/writes wait at most getOption("timeout") like base R sockets */
static double sock_timeout_ms(void) {
  double timeout = Rf_asReal(Rf_GetOption1(Rf_install("timeout")));
  return (R_FINITE(timeout) && timeout > 0) ? timeout * 1000 : 0;
}

static size_t sock_read(void *target, size_t req_size, Rconnection con) {
  request *req = (request*) con->private;
  curl_socket_t sockfd = get_active_socket(req);
  double timeout_ms = sock_timeout_ms();
  double waited_ms = 0;
  size_t n = 0;
  CURLcode rc = curl_easy_recv(req->handle, target, req_size, &n);
  while(rc == CURLE_AGAIN) {
    if(!con->blocking || (timeout_ms > 0 && waited_ms >= timeout_ms)) {
      /* no data available (yet): non-blocking read or timeout reached */
      con->incomplete = TRUE;
      return 0;
    }
    if(pending_interrupt())
      assert_message(CURLE_ABORTED_BY_CALLBACK, NULL);
    if(wait_on_socket(sockfd, 1, 500) < 0)
      Rf_error("Failure in select() while waiting for data");
    waited_ms += 500;
    rc = curl_easy_recv(req->handle, target, req_size, &n);
  }
  assert_status(rc, req->ref);
  /* zero bytes with CURLE_OK means the peer closed the connection */
  con->incomplete = n > 0;
  return n;
}

static size_t rcurl_write(const void *buf, size_t sz, size_t ni, Rconnection con) {
  request *req = (request*) con->private;
  if(!req->connect_only)
    Rf_error("Connection is not writable");
  curl_socket_t sockfd = get_active_socket(req);
  double timeout_ms = sock_timeout_ms();
  double waited_ms = 0;
  size_t total = sz * ni;
  size_t sent = 0;
  while(sent < total) {
    size_t n = 0;
    CURLcode rc = curl_easy_send(req->handle, (const char*) buf + sent, total - sent, &n);
    if(rc == CURLE_AGAIN) {
      if(timeout_ms > 0 && waited_ms >= timeout_ms)
        Rf_error("Timeout was reached sending data on connection");
      if(pending_interrupt())
        assert_message(CURLE_ABORTED_BY_CALLBACK, NULL);
      if(wait_on_socket(sockfd, 0, 500) < 0)
        Rf_error("Failure in select() while waiting to send");
      waited_ms += 500;
      continue;
    }
    assert_status(rc, req->ref);
    sent += n;
  }
  return ni;
}

static size_t rcurl_read(void *target, size_t sz, size_t ni, Rconnection con) {
  request *req = (request*) con->private;
  size_t req_size = sz * ni;
  if(req->connect_only)
    return sock_read(target, req_size, con);
  size_t total_size = pop(target, req_size, req);
  if (total_size > 0 && (!con->blocking || req->partial)) {
      // If we can return data without waiting, and the connection is
      // non-blocking (or using curl_fetch_stream()), do so.
      // This ensures that bytes we already received get flushed
      // to the target buffer before a connection error.
      con->incomplete = req->has_more || req->size;
      return total_size;
  }

  while((req_size > total_size) && req->has_more) {
    int numfds;
    massert(curl_multi_wait(req->manager, NULL, 0, con->blocking ? 1000 : 10, &numfds));

    fetchdata(req);
    total_size += pop((char*)target + total_size, (req_size-total_size), req);

    //return less than requested data for non-blocking connections, or curl_fetch_stream()
    if(!con->blocking || req->partial)
      break;
  }
  con->incomplete = req->has_more || req->size;
  return total_size;
}

static int rcurl_fgetc(Rconnection con) {
  int x = 0;
#ifdef WORDS_BIGENDIAN
  return rcurl_read(&x, 1, 1, con) ? BSWAP_32(x) : R_EOF;
#else
  return rcurl_read(&x, 1, 1, con) ? x : R_EOF;
#endif
}

static void cleanup(Rconnection con) {
  request *req = (request*) con->private;
  reference *ref = req->ref;

  /* free thee handle connection */
  curl_multi_remove_handle(req->manager, req->handle);
  curl_easy_setopt(req->handle, CURLOPT_WRITEFUNCTION, NULL);
  curl_easy_setopt(req->handle, CURLOPT_WRITEDATA, NULL);
  curl_easy_setopt(req->handle, CURLOPT_FAILONERROR, 0L);
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
static void reset(Rconnection con) {
  request *req = (request*) con->private;
  curl_multi_remove_handle(req->manager, req->handle);
  curl_easy_setopt(req->handle, CURLOPT_WRITEFUNCTION, NULL);
  curl_easy_setopt(req->handle, CURLOPT_WRITEDATA, NULL);
  curl_easy_setopt(req->handle, CURLOPT_FAILONERROR, 0L);
  curl_easy_setopt(req->handle, CURLOPT_CONNECT_ONLY, 0L);
  req->connect_only = 0;
  req->ref->locked = 0;
  con->isopen = FALSE;
  con->canwrite = FALSE;
  con->text = TRUE;
  con->incomplete = FALSE;
  strcpy(con->mode, "r");
}

static Rboolean rcurl_open(Rconnection con) {
  request *req = (request*) con->private;

  //same message as base::url()
  if (con->mode[0] != 'r' || strchr(con->mode, 'w'))
    Rf_error("can only open URLs for reading");

  if(req->ref->locked)
    Rf_error("Handle is already in use elsewhere.");

  /* mode with '+' means a bidirectional (connect-only) socket connection */
  req->connect_only = strchr(con->mode, '+') != NULL;

  /* init a multi stack with callback */
  CURL *handle = req->handle;
  assert(curl_easy_setopt(handle, CURLOPT_URL, req->url));
  assert(curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, push));
  assert(curl_easy_setopt(handle, CURLOPT_WRITEDATA, req));
  assert(curl_easy_setopt(handle, CURLOPT_CONNECT_ONLY, req->connect_only ? 1L : 0L));

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

  /* fully non-blocking has 's' in open mode */
  int block_open = strchr(con->mode, 's') == NULL;
  int force_open = strchr(con->mode, 'f') != NULL;
  if(block_open && !force_open)
    curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1L);

 /* Wait for first data to arrive. Monitoring a change in status code does not
   suffice in case of http redirects */
  while(block_open && req->has_more && !req->has_data) {
    int numfds;
    massert(curl_multi_wait(req->manager, NULL, 0, 1000, &numfds));
    if(pending_interrupt()) {
      reset(con); //cleanup before jumping
      assert_message(CURLE_ABORTED_BY_CALLBACK, NULL);
    }
    massert(curl_multi_perform(req->manager, &(req->has_more)));
    for(int msg = 1; msg > 0;){
      CURLMsg *out = curl_multi_info_read(req->manager, &msg);
      if(out && out->data.result != CURLE_OK){
        const char *errmsg = strlen(req->ref->errbuf) ? req->ref->errbuf : curl_easy_strerror(out->data.result);
        Rf_warningcall(R_NilValue, "Failed to open '%s': %s", req->url, errmsg);
        reset(con);
        return FALSE;
      }
    }
  }

  /* check http status code */
  /* Stream connections should be checked via handle_data() */
  /* Non-blocking open connections get checked during read */

  /* connect-only handles are done after the connect: verify we got a socket */
  if(req->connect_only) {
    curl_socket_t sockfd = CURL_SOCKET_BAD;
    assert(curl_easy_getinfo(handle, CURLINFO_ACTIVESOCKET, &sockfd));
    if(sockfd == CURL_SOCKET_BAD) {
      Rf_warningcall(R_NilValue, "Failed to connect to '%s'", req->url);
      reset(con);
      return FALSE;
    }
    con->canwrite = TRUE;
  }

  /* set mode in case open() changed it */
  con->text = strchr(con->mode, 'b') ? FALSE : TRUE;
  con->isopen = TRUE;
  con->incomplete = TRUE;
  return TRUE;
}

SEXP R_curl_connection(SEXP url, SEXP ptr, SEXP partial) {
  if(!Rf_isString(url))
    Rf_error("Argument 'url' must be string.");

  /* create the R connection object, mimicking base::url() */
  Rconnection con;

  /* R wants description in native encoding, but we use UTF-8 URL below */
  SEXP rc = PROTECT(R_new_custom_connection(Rf_translateChar(STRING_ELT(url, 0)), "r", "curl", &con));

  /* setup curl. These are the parts that are recyclable. */
  request *req = malloc(sizeof(request));
  req->handle = get_handle(ptr);
  req->ref = get_ref(ptr);
  req->limit = CURL_MAX_WRITE_SIZE;
  req->buf = malloc(req->limit);
  req->manager = curl_multi_init();
  req->partial = Rf_asLogical(partial); //only for curl_fetch_stream()
  req->used = 0;
  req->connect_only = 0;

  /* allocate url string */
  req->url = malloc(strlen(Rf_translateCharUTF8(Rf_asChar(url))) + 1);
  strcpy(req->url, Rf_translateCharUTF8(Rf_asChar(url)));

  /* set connection properties */
  con->incomplete = FALSE;
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
  con->write = rcurl_write;
  con->fgetc = rcurl_fgetc;
  con->fgetc_internal = rcurl_fgetc;

  /* protect the handle */
  (req->ref->refCount)++;

  /* store the CURLM address in con->ex_ptr which is the 'conn_id' attribute */
  R_SetExternalPtrAddr((SEXP) con->ex_ptr, req->manager);

  UNPROTECT(1);
  return rc;
}
