/* Hack to always include the typechecking macros */
#include <curl/curl.h>
#ifndef __CURL_TYPECHECK_GCC_H
#include <curl/typecheck-gcc.h>
#endif

int r_curl_is_slist_option(CURLoption x){
  return _curl_is_slist_option(x);
}

int r_curl_is_long_option(CURLoption x){
  return _curl_is_long_option(x);
}

int r_curl_is_off_t_option(CURLoption x){
  return _curl_is_off_t_option(x);
}

int r_curl_is_string_option(CURLoption x){
  return _curl_is_string_option(x);
}

int r_curl_is_postfields_option(CURLoption x){
  return _curl_is_postfields_option(x);
}
