#include <R_ext/Rdynload.h>
#include <curl/curl.h>

CURLM *multi_handle = NULL;

void R_init_curl(DllInfo *info) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  multi_handle = curl_multi_init();
}

void R_unload_curl(DllInfo *info) {
  curl_multi_cleanup(multi_handle);
  curl_global_cleanup();
}
