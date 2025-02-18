/* *
 * Blocking easy interfaces to libcurl for R.
 * Example: https://curl.se/libcurl/c/getinmemory.html
 */

#include "curl-common.h"

static size_t write_nothing(void *contents, size_t sz, size_t nmemb, void *ctx) {
  return sz * nmemb;
}

static void run_httpuv(void *dummy) {
  SEXP expr = PROTECT(Rf_lang1(Rf_install("later_wrapper")));
  SEXP env = PROTECT(R_FindNamespace(Rf_mkString("curl")));
  Rf_eval(expr, env);
  UNPROTECT(2);
}

static int process_server(void) {
  return !(R_ToplevelExec(run_httpuv, NULL));
}

SEXP R_curl_dryrun(SEXP ptr){
  CURL *handle = get_handle(ptr);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_nothing);
  CURLM * multi_handle = curl_multi_init();
  if(CURLM_OK != curl_multi_add_handle(multi_handle, handle))
    goto cleanup;
  int still_running = 1;
  while(still_running) {
    if(process_server())
      break;
    if(curl_multi_perform(multi_handle, &(still_running)) != CURLM_OK)
      break;
  }
  cleanup:
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, NULL);
    curl_multi_remove_handle(multi_handle, handle);
    curl_multi_cleanup(multi_handle);
  return R_NilValue;
}
