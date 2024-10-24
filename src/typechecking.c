#include "curl-common.h"

int r_curl_ignore_option(CURLoption x){
  switch (x) {
  case CURLOPT_POSTFIELDSIZE:
  case CURLOPT_POSTFIELDSIZE_LARGE:
    return 1;
  default:
    return 0;
  }
}

#ifdef HAS_CURL_EASY_OPTION

int r_curl_is_string_option(CURLoption x){
  return curl_easy_option_by_id(x)->type == CURLOT_STRING;
}

int r_curl_is_slist_option(CURLoption x){
  return curl_easy_option_by_id(x)->type == CURLOT_SLIST;
}

int r_curl_is_long_option(CURLoption x){
  const curl_easytype type = curl_easy_option_by_id(x)->type;
  return type == CURLOT_LONG || type == CURLOT_VALUES;
}

int r_curl_is_off_t_option(CURLoption x){
  return curl_easy_option_by_id(x)->type == CURLOT_OFF_T;
}

int r_curl_is_postfields_option(CURLoption x){
  return curl_easy_option_by_id(x)->type == CURLOT_OBJECT;
}

#else //CURLOT_FLAG_ALIAS

#include "typelist.h"

int r_curl_is_string_option(CURLoption x){
  return curlcheck_string_option(x);
}

int r_curl_is_slist_option(CURLoption x){
  return curlcheck_slist_option(x);
}

int r_curl_is_long_option(CURLoption x){
  return curlcheck_long_option(x);
}

int r_curl_is_off_t_option(CURLoption x){
  return curlcheck_off_t_option(x);
}

int r_curl_is_postfields_option(CURLoption x){
  return curlcheck_postfields_option(x);
}

#endif //CURLOT_FLAG_ALIAS
