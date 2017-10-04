#include <R_ext/Rdynload.h>
#include <curl/curl.h>

void select_ssl_backend();
CURLM *multi_handle = NULL;
static struct curl_slist * default_headers = NULL;

void R_init_curl(DllInfo *info) {
  select_ssl_backend();
  curl_global_init(CURL_GLOBAL_DEFAULT);
  multi_handle = curl_multi_init();
  default_headers = curl_slist_append(default_headers, "Expect:");
  R_registerRoutines(info, NULL, NULL, NULL, NULL);
  R_useDynamicSymbols(info, TRUE);
}

void R_unload_curl(DllInfo *info) {
  curl_multi_cleanup(multi_handle);
  curl_global_cleanup();
}
