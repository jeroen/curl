/* MacOS-11 SDK headers are older than it's runtime, so we add this polyfill  */
#ifndef CURLINC_OPTIONS_H
#define CURLINC_OPTIONS_H
typedef enum {
  CURLOT_LONG,
  CURLOT_VALUES,
  CURLOT_OFF_T,
  CURLOT_OBJECT,
  CURLOT_STRING,
  CURLOT_SLIST,
  CURLOT_CBPTR,
  CURLOT_BLOB,
  CURLOT_FUNCTION
} curl_easytype;

#define CURLOT_FLAG_ALIAS (1<<0)
struct curl_easyoption {
  const char *name;
  CURLoption id;
  curl_easytype type;
  unsigned int flags;
};

CURL_EXTERN const struct curl_easyoption *
curl_easy_option_by_name(const char *name);

CURL_EXTERN const struct curl_easyoption *
curl_easy_option_by_id(CURLoption id);

CURL_EXTERN const struct curl_easyoption *
curl_easy_option_next(const struct curl_easyoption *prev);

#ifdef __cplusplus
#endif
#endif