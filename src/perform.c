/* *
 * Easy interface to libcurl for R. (c) 2015 Jeroen Ooms.
 * Example: http://curl.haxx.se/libcurl/c/getinmemory.html
 */

#include <curl/curl.h>
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
  if (pending_interrupt())
    return 0;

  /* avoids compiler warning on windows */
  size_t realsize = sz * nmemb;
  memory *mem = (memory*) ctx;

  /* increase buffer size */
  mem->buf = realloc(mem->buf, mem->size + realsize);
  if (!mem->buf)
    return 0;

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
  assert(curl_easy_getinfo(handle, CURLINFO_COOKIELIST, &cookies));
  SEXP out = slist_to_vec(cookies);
  curl_slist_free_all(cookies);
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
  SEXP names = PROTECT(allocVector(STRSXP, 6));
  SET_STRING_ELT(names, 0, mkChar("url"));
  SET_STRING_ELT(names, 1, mkChar("status_code"));
  SET_STRING_ELT(names, 2, mkChar("headers"));
  SET_STRING_ELT(names, 3, mkChar("content"));
  SET_STRING_ELT(names, 4, mkChar("modified"));
  SET_STRING_ELT(names, 5, mkChar("times"));
  UNPROTECT(1);
  return names;
}

SEXP R_curl_fetch_memory(SEXP url, SEXP ptr){
  if (!isString(url) || length(url) != 1)
    error("Argument 'url' must be string.");

  /* get the handle */
  CURL *handle = get_handle(ptr);

  /* update the url */
  curl_easy_setopt(handle, CURLOPT_URL, translateCharUTF8(asChar(url)));

  /* buffer body and headers */
  memory body = {NULL, 0};
  memory headers = {NULL, 0};
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, append_buffer);
  curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, append_buffer);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, &body);
  curl_easy_setopt(handle, CURLOPT_HEADERDATA, &headers);

  /* perform blocking request */
  CURLcode status = curl_easy_perform(handle);

  /* Reset for reuse */
  curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, NULL);
  curl_easy_setopt(handle, CURLOPT_HEADERDATA, NULL);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, NULL);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, NULL);

  /* check for errors */
  if (status != CURLE_OK) {
    free(body.buf);
    free(headers.buf);
    error(curl_easy_strerror(status));
  }

  /* create output */
  SEXP res = PROTECT(allocVector(VECSXP, 6));
  SET_VECTOR_ELT(res, 0, make_url(handle));
  SET_VECTOR_ELT(res, 1, make_status(handle));
  SET_VECTOR_ELT(res, 2, make_rawvec(headers.buf, headers.size));
  SET_VECTOR_ELT(res, 3, make_rawvec(body.buf, body.size));
  SET_VECTOR_ELT(res, 4, make_filetime(handle));
  SET_VECTOR_ELT(res, 5, make_timevec(handle));
  setAttrib(res, R_NamesSymbol, make_namesvec());
  UNPROTECT(1);

  free(body.buf);
  free(headers.buf);

  return res;
}

SEXP R_curl_fetch_disk(SEXP url, SEXP ptr, SEXP path, SEXP mode){
  if (!isString(url) || length(url) != 1)
    error("Argument 'url' must be string.");
  if (!isString(path) || length(path) != 1)
    error("`path` must be string.");


  /* get the handle */
  CURL *handle = get_handle(ptr);

  /* update the url */
  curl_easy_setopt(handle, CURLOPT_URL, translateCharUTF8(asChar(url)));

  /* buffer body and headers */
  memory headers = {NULL, 0};
  curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, append_buffer);
  curl_easy_setopt(handle, CURLOPT_HEADERDATA, &headers);

  /* open file */
  FILE *dest = fopen(translateCharUTF8(asChar(path)), CHAR(asChar(mode)));
  if(!dest)
    error("Failed to open file %s.", translateCharUTF8(asChar(path)));
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, push_disk);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, dest);

  /* perform blocking request */
  CURLcode status = curl_easy_perform(handle);

  /* cleanup */
  curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, NULL);
  curl_easy_setopt(handle, CURLOPT_HEADERDATA, NULL);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, NULL);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, NULL);
  fclose(dest);

  /* check for errors */
  if (status != CURLE_OK) {
    free(headers.buf);
    error(curl_easy_strerror(status));
  }

  /* create output */
  SEXP res = PROTECT(allocVector(VECSXP, 6));
  SET_VECTOR_ELT(res, 0, make_url(handle));
  SET_VECTOR_ELT(res, 1, make_status(handle));
  SET_VECTOR_ELT(res, 2, make_rawvec(headers.buf, headers.size));
  SET_VECTOR_ELT(res, 3, path);
  SET_VECTOR_ELT(res, 4, make_filetime(handle));
  SET_VECTOR_ELT(res, 5, make_timevec(handle));
  setAttrib(res, R_NamesSymbol, make_namesvec());

  /* cleanup */
  UNPROTECT(1);
  return res;
}

SEXP R_get_handle_cookies(SEXP ptr){
  return make_cookievec(get_handle(ptr));
}
