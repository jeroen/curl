/* Proposed addition to the 'curl' R package: src/socket.c
 * Raw send/recv on a handle connected with CURLOPT_CONNECT_ONLY.
 * Uses the package's own handle accessor (get_handle() in curl-common.h). */
#include "curl-common.h"
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/select.h>
#include <sys/time.h>
#endif

static int wait_on_socket(curl_socket_t sockfd, int for_recv, long timeout_ms) {
  struct timeval tv;
  fd_set infd, outfd, errfd;
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;
  FD_ZERO(&infd); FD_ZERO(&outfd); FD_ZERO(&errfd);
  FD_SET(sockfd, &errfd);
  if (for_recv) FD_SET(sockfd, &infd); else FD_SET(sockfd, &outfd);
  return select((int) sockfd + 1, &infd, &outfd, &errfd, &tv);
}

/* Establish the connection (TCP + TLS) without speaking the protocol.
 * curl_easy_perform() is called directly on the easy handle: the regular
 * fetch path performs on the shared multi handle and removes the easy handle
 * afterwards (interrupt.c), which leaves a CONNECT_ONLY handle without its
 * connection, so curl_easy_send()/recv() would fail. */
SEXP R_curl_connect(SEXP ptr, SEXP url) {
  reference *ref = get_ref(ptr);
  assert(curl_easy_setopt(ref->handle, CURLOPT_URL, get_string(url)));
  assert(curl_easy_setopt(ref->handle, CURLOPT_CONNECT_ONLY, 1L));
  reset_errbuf(ref);
  assert_status(curl_easy_perform(ref->handle), ref);
  return R_NilValue;
}

SEXP R_curl_send(SEXP ptr, SEXP data, SEXP timeout_ms) {
  CURL *handle = get_handle(ptr);
  curl_socket_t sockfd;
  assert(curl_easy_getinfo(handle, CURLINFO_ACTIVESOCKET, &sockfd));
  const unsigned char *buf = RAW(data);
  size_t len = (size_t) Rf_xlength(data), total = 0, n = 0;
  long tmo = (long) Rf_asInteger(timeout_ms);
  while (total < len) {
    CURLcode rc = curl_easy_send(handle, buf + total, len - total, &n);
    if (rc == CURLE_AGAIN) {
      int w = wait_on_socket(sockfd, 0, tmo);
      if (w == 0) Rf_error("timeout while sending");
      if (w < 0) Rf_error("socket error while sending");
      continue;
    }
    assert(rc);
    total += n;
  }
  return Rf_ScalarInteger((int) total);
}

SEXP R_curl_recv(SEXP ptr, SEXP timeout_ms, SEXP max_bytes) {
  CURL *handle = get_handle(ptr);
  curl_socket_t sockfd;
  assert(curl_easy_getinfo(handle, CURLINFO_ACTIVESOCKET, &sockfd));
  long tmo = (long) Rf_asInteger(timeout_ms);
  size_t cap = (size_t) Rf_asInteger(max_bytes), n = 0;
  if (cap < 1) cap = 1;
  unsigned char *buf = (unsigned char *) R_alloc(cap, 1);
  CURLcode rc = curl_easy_recv(handle, buf, cap, &n);
  if (rc == CURLE_AGAIN) {
    int w = wait_on_socket(sockfd, 1, tmo);
    if (w == 0) return Rf_allocVector(RAWSXP, 0);
    if (w < 0) Rf_error("socket error while receiving");
    rc = curl_easy_recv(handle, buf, cap, &n);
    if (rc == CURLE_AGAIN) return Rf_allocVector(RAWSXP, 0);
  }
  assert(rc);
  SEXP out = PROTECT(Rf_allocVector(RAWSXP, n));
  if (n > 0) memcpy(RAW(out), buf, n);
  if (n == 0) Rf_setAttrib(out, Rf_install("closed"), Rf_ScalarLogical(1));
  UNPROTECT(1);
  return out;
}
