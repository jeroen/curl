#include <R_ext/Rdynload.h>
#include <curl/curl.h>

void R_init_curl(DllInfo *info) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

void R_unload_curl(DllInfo *info) {
  curl_global_cleanup();
}
