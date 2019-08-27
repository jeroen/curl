#include <curl/curl.h>

/* Hack to get the GCC macros on other systems */
#if !defined(__CURL_TYPECHECK_GCC_H) || !defined(CURLINC_TYPECHECK_GCC_H)
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

/* New names */
#ifdef CURLINC_TYPECHECK_GCC_H
#define _curl_is_string_option curlcheck_string_option
#define _curl_is_slist_option curlcheck_slist_option
#define _curl_is_off_t_option curlcheck_off_t_option
#define _curl_is_long_option curlcheck_long_option
#define _curl_is_postfields_option curlcheck_postfields_option
#define _curl_is_cb_data_option curlcheck_cb_data_option
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

/* workaround old curl bug: if an option is neither classified as string
 * stringlist, we assume string: https://github.com/jeroen/curl/issues/192
 */
int r_curl_is_string_option(CURLoption x){
  return _curl_is_string_option(x)  ||
    (x > 10000 && x < 20000 && !_curl_is_slist_option(x) && !_curl_is_cb_data_option(x));
}

int r_curl_is_postfields_option(CURLoption x){
  return _curl_is_postfields_option(x);
}
