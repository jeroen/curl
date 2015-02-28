#include <curl/curl.h>
#include <Rinternals.h>
#include "utils.h"

void fin_handle(SEXP ptr){
  if(!R_ExternalPtrAddr(ptr)) return;
  curl_easy_cleanup(R_ExternalPtrAddr(ptr));
  R_ClearExternalPtr(ptr);
}

CURL *make_handle(){
  /* construct new handler */
  CURL *http_handle = curl_easy_init();

  /* aka 'CURLOPT_ACCEPT_ENCODING' in recent versions */
  curl_easy_setopt(http_handle, CURLOPT_ENCODING, "gzip, deflate");

  /* needed to start the cookie engine */
  curl_easy_setopt(http_handle, CURLOPT_COOKIEFILE, "");
  curl_easy_setopt(http_handle, CURLOPT_FILETIME, 1);

  /*return the handler */
  return http_handle;
}

SEXP R_new_handle(){
  CURL *handle = make_handle();
  SEXP ptr = PROTECT(R_MakeExternalPtr(handle, R_NilValue, R_NilValue));
  R_RegisterCFinalizerEx(ptr, fin_handle, 1);
  setAttrib(ptr, R_ClassSymbol, mkString("curl_handle"));
  UNPROTECT(1);
  return ptr;
}

SEXP R_handle_setopt(SEXP ptr, SEXP keys, SEXP values){
  CURL *handle = get_handle(ptr);
  SEXP optnames = getAttrib(values, R_NamesSymbol);

  if(!isInteger(keys))
    error("Argument 'names' must be numeric.");

  if(!isVector(values))
    error("Argument 'values' must be List.");

  for(int i = 0; i < length(keys); i++){
    int key = INTEGER(keys)[i];
    const char* optname = CHAR(STRING_ELT(optnames, i));
    SEXP val = VECTOR_ELT(values, i);
    if(key < 10000){
      if(!isNumeric(val)){
        error("Value for %s (%d) must be numeric.", optname, key);
      }
      assert(curl_easy_setopt(handle, key, asInteger(val)));
    } else if(key < 20000){
      if(!isString(val)){
        error("Value for %s (%d) must be a string.", optname, key);
      }
      assert(curl_easy_setopt(handle, key, CHAR(asChar(val))));
    } else {
      error("Option %d not supported.", key);
    }
  }

  return ScalarLogical(1);
}

SEXP R_handle_setheader(SEXP ptr, SEXP vec){
  CURL *handle = get_handle(ptr);
  if(!isString(vec))
    error("header vector must be a string.");
  assert(curl_easy_setopt(handle, CURLOPT_HTTPHEADER, vec_to_slist(vec)));
  return ScalarLogical(1);
}
