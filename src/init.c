#include <R_ext/Rdynload.h>
#include <curl/curl.h>

CURLM *global_multi;

void R_init_curl(DllInfo *info) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  global_multi = curl_multi_init();
}

void R_unload_curl(DllInfo *info) {
  curl_multi_cleanup(global_multi);
  curl_global_cleanup();
}
