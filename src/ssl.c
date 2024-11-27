#include "curl-common.h"

void select_tls_backend(void){
#if defined(_WIN32) && AT_LEAST_CURL(8,0)
  /* If a CURL_SSL_BACKEND is set, do not override */
  char *envvar = getenv("CURL_SSL_BACKEND");
  if(envvar != NULL && *envvar != 0) {
    REprintf("Initiating curl with CURL_SSL_BACKEND: %s\n", envvar);
    return;
  }

  /* Default to using OpenSSL (which supports http/2) */
  switch(curl_global_sslset(CURLSSLBACKEND_OPENSSL, NULL, NULL)) {
    case CURLSSLSET_OK :
      break;
    case CURLSSLSET_TOO_LATE:
      Rf_warning("Failed to set libcurl SSL: already initiated");
      break;
    case CURLSSLSET_UNKNOWN_BACKEND:
      Rf_warning("Failed to set libcurl SSL: unsupported backend");
      break;
    default:
      Rf_warning("Failed to set libcurl SSL: unknown error");
      break;
  }

#endif
}
