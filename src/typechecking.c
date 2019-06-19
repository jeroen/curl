/* Hack to get the GCC macros on all systems */
#include <curl/curl.h>
#ifndef __CURL_TYPECHECK_GCC_H
#ifdef __warning__
#undef __warning__
#endif
#define __warning__(x)
#ifdef __unused__
#undef __unused__
#endif
#define __unused__
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
