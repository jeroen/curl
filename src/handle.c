#include <curl/curl.h>
#include <curl/easy.h>
#include <Rinternals.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

void fin_handle(SEXP ptr){
  if(!R_ExternalPtrAddr(ptr)) return;
  curl_easy_cleanup(R_ExternalPtrAddr(ptr));
  R_ClearExternalPtr(ptr);
}

SEXP R_new_handle(){
  CURL *handle = make_handle(NULL);
  SEXP ptr = PROTECT(R_MakeExternalPtr(handle, R_NilValue, R_NilValue));
  R_RegisterCFinalizerEx(ptr, fin_handle, 1);
  setAttrib(ptr, R_ClassSymbol, mkString("curl_handle"));
  UNPROTECT(1);
  return ptr;
}
