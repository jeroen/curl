/* *
 * Easy interface to libcurl for R. (c) 2015 Jeroen Ooms.
 * Example: http://curl.haxx.se/libcurl/c/getinmemory.html
 */

#include <curl/curl.h>
#include <curl/easy.h>
#include <Rinternals.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

/* Adapted from curl example */
typedef struct {
  unsigned char *buf;
  size_t size;
} memory;

static size_t append_buffer(void *contents, size_t sz, size_t nmemb, void *ctx) {
  /* avoids compiler warning on windows */
  size_t realsize = sz * nmemb;
  memory *mem = (memory*) ctx;

  /* increase buffer size */
  mem->buf = realloc(mem->buf, mem->size + realsize);
  if(!mem->buf)
    error("Out of memory.");

  /* append data and increment size */
  memcpy(&(mem->buf[mem->size]), contents, realsize);
  mem->size += realsize;
  return realsize;
}

SEXP make_timevec(CURL *handle){
  double time_redirect, time_lookup, time_connect, time_pre, time_start, time_total;
  assert(curl_easy_getinfo(handle, CURLINFO_REDIRECT_TIME, &time_redirect));
  assert(curl_easy_getinfo(handle, CURLINFO_NAMELOOKUP_TIME, &time_lookup));
  assert(curl_easy_getinfo(handle, CURLINFO_CONNECT_TIME, &time_connect));
  assert(curl_easy_getinfo(handle, CURLINFO_PRETRANSFER_TIME, &time_pre));
  assert(curl_easy_getinfo(handle, CURLINFO_STARTTRANSFER_TIME, &time_start));
  assert(curl_easy_getinfo(handle, CURLINFO_TOTAL_TIME, &time_total));

  SEXP result = PROTECT(allocVector(REALSXP, 6));
  REAL(result)[0] = time_redirect;
  REAL(result)[1] = time_lookup;
  REAL(result)[2] = time_connect;
  REAL(result)[3] = time_pre;
  REAL(result)[4] = time_start;
  REAL(result)[5] = time_total;

  SEXP names = PROTECT(allocVector(STRSXP, 6));
  SET_STRING_ELT(names, 0, mkChar("redirect"));
  SET_STRING_ELT(names, 1, mkChar("namelookup"));
  SET_STRING_ELT(names, 2, mkChar("connect"));
  SET_STRING_ELT(names, 3, mkChar("pretransfer"));
  SET_STRING_ELT(names, 4, mkChar("starttransfer"));
  SET_STRING_ELT(names, 5, mkChar("total"));
  setAttrib(result, R_NamesSymbol, names);
  UNPROTECT(2);
  return result;
}


/* Extract current cookies (state) from handle */
SEXP make_cookievec(CURL *handle){
  /* linked list of strings */
  struct curl_slist *cookies;
  struct curl_slist *cursor;
  assert(curl_easy_getinfo(handle, CURLINFO_COOKIELIST, &cookies));

  /* count cookies */
  int n = 0;
  cursor = cookies;
  while (cursor) {
    n++;
    cursor = cursor->next;
  }

  SEXP out = PROTECT(allocVector(STRSXP, n));
  cursor = cookies;
  for(int i = 0; i < n; i++){
    SET_STRING_ELT(out, i, mkChar(cursor->data));
    cursor = cursor->next;
  }
  curl_slist_free_all(cookies);
  UNPROTECT(1);
  return out;
}

SEXP make_status(CURL *handle){
  long res_status;
  assert(curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &res_status));
  return ScalarInteger(res_status);
}

SEXP make_url(CURL *handle){
  char *res_url;
  assert(curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &res_url));
  return mkString(res_url);
}

SEXP make_filetime(CURL *handle){
  long filetime;
  assert(curl_easy_getinfo(handle, CURLINFO_FILETIME, &filetime));
  if(filetime < 0){
    filetime = NA_INTEGER;
  }

  SEXP classes = PROTECT(allocVector(STRSXP, 2));
  SET_STRING_ELT(classes, 0, mkChar("POSIXct"));
  SET_STRING_ELT(classes, 1, mkChar("POSIXt"));

  SEXP out = PROTECT(ScalarInteger(filetime));
  setAttrib(out, R_ClassSymbol, classes);
  UNPROTECT(2);
  return out;
}

SEXP make_rawvec(unsigned char *ptr, size_t size){
  SEXP out = PROTECT(allocVector(RAWSXP, size));
  memcpy(RAW(out), ptr, size);
  UNPROTECT(1);
  return out;
}

SEXP make_namesvec(){
  SEXP names = PROTECT(allocVector(STRSXP, 8));
  SET_STRING_ELT(names, 0, mkChar("url"));
  SET_STRING_ELT(names, 1, mkChar("status_code"));
  SET_STRING_ELT(names, 2, mkChar("headers"));
  SET_STRING_ELT(names, 3, mkChar("cookies"));
  SET_STRING_ELT(names, 4, mkChar("content"));
  SET_STRING_ELT(names, 5, mkChar("modified"));
  SET_STRING_ELT(names, 6, mkChar("times"));
  SET_STRING_ELT(names, 7, mkChar("request"));
  UNPROTECT(1);
  return names;
}

SEXP R_curl_perform(SEXP url){
  if(!isString(url))
    error("Argument 'url' must be string.");

  /* create generic handle */
  CURL *handle = make_handle(translateCharUTF8(asChar(url)));

  /* buffer body output */
  memory body = {NULL, 0};
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, append_buffer);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, &body);

  /* buffer response headers */
  memory headers = {NULL, 0};
  curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, append_buffer);
  curl_easy_setopt(handle, CURLOPT_HEADERDATA, &headers);

  /* perform blocking request */
  assert(curl_easy_perform(handle));

  /* create output */
  SEXP res = PROTECT(allocVector(VECSXP, 8));
  SET_VECTOR_ELT(res, 0, make_url(handle));
  SET_VECTOR_ELT(res, 1, make_status(handle));
  SET_VECTOR_ELT(res, 2, make_rawvec(headers.buf, headers.size));
  SET_VECTOR_ELT(res, 3, make_cookievec(handle));
  SET_VECTOR_ELT(res, 4, make_rawvec(body.buf, body.size));
  SET_VECTOR_ELT(res, 5, make_filetime(handle));
  SET_VECTOR_ELT(res, 6, make_timevec(handle));
  SET_VECTOR_ELT(res, 7, R_NilValue);
  setAttrib(res, R_NamesSymbol, make_namesvec());

  /* cleanup */
  UNPROTECT(1);
  free(body.buf);
  free(headers.buf);
  curl_easy_cleanup(handle);
  return res;
}
