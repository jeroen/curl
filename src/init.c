#include <R_ext/Rdynload.h>
#include <curl/curl.h>

/* defined in multi.c */
void global_multi_init();
void global_multi_cleanup();

void R_init_curl(DllInfo *info) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  global_multi_init();
}

void R_unload_curl(DllInfo *info) {
  global_multi_cleanup();
}
