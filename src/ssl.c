#include <curl/curl.h>
#include <Rinternals.h>

#if LIBCURL_VERSION_MAJOR > 7 || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR >= 56)
#define HAS_MULTI_SSL 1
#endif

/* Fall back on OpenSSL on Legacy Windows (Vista/2008) which do not support TLS 1.2 natively */
void select_ssl_backend(){
#if defined(_WIN32) && defined(HAS_MULTI_SSL)
  DWORD dwBuild = 0;
  DWORD dwVersion = GetVersion();
  if (dwVersion < 0x80000000)
    dwBuild = (DWORD)(HIWORD(dwVersion));

  /* TLS 1.2 requires at least Windows 7 or 2008-R2 */
  curl_sslbackend backend = dwBuild < 7600 ? CURLSSLBACKEND_OPENSSL : CURLSSLBACKEND_SCHANNEL;

  /* Try to set the backend */
  switch(curl_global_sslset(backend, NULL, NULL)){
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
