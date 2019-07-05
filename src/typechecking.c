/* Hack to get the GCC macros on all systems */
#include <curl/curl.h>
#ifndef __CURL_TYPECHECK_GCC_H
#undef curl_easy_setopt
#undef curl_easy_getinfo
#undef curl_share_setopt
#undef curl_multi_setopt

/* Remove the GNU extensions from typecheck-gcc.h */
#ifndef __warning__
#define __warning__(x)
#endif
#ifndef __unused__
#define __unused__
#endif

/* Add the file */
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
