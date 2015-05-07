#include <curl/curl.h>
#include <Rinternals.h>
#include <stdlib.h>
#include "utils.h"
#include "callbacks.h"

void clean_handle(reference *ref){
  if(ref->garbage && !(ref->inUse)){
    //Rprintf("cleaning easy handle\n");
    if(ref->headers)
      curl_slist_free_all(ref->headers);
    if(ref->form)
      curl_formfree(ref->form);
    if(ref->handle)
      curl_easy_cleanup(ref->handle);
    free(ref);
  }
}

void fin_handle(SEXP ptr){
  //Rprintf("finalizing handle\n");
  reference *ref = (reference*) R_ExternalPtrAddr(ptr);
  if(ref){
    ref->garbage = 1;
    clean_handle(ref);
  }
  R_ClearExternalPtr(ptr);
}

/* These are defaulst that we always want to set */
void set_handle_defaults(CURL *handle){

  /* needed to support compressed responses */
  assert(curl_easy_setopt(handle, CURLOPT_ENCODING, "gzip, deflate"));

  /* do not validate SSL certificates by default */
  assert(curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L));
  assert(curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L));

  /* follow redirect */
  assert(curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L));
  assert(curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 10L));

  /* a sensible timeout (10s) */
  assert(curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT_MS, 10000L));

  /* needed to start the cookie engine */
  assert(curl_easy_setopt(handle, CURLOPT_COOKIEFILE, ""));
  assert(curl_easy_setopt(handle, CURLOPT_FILETIME, 1L));

  /* a default user agent */
  assert(curl_easy_setopt(handle, CURLOPT_USERAGENT, "r/curl/jeroen"));

  /* allow all authentication methods */
  assert(curl_easy_setopt(handle, CURLOPT_HTTPAUTH, CURLAUTH_ANY));
  assert(curl_easy_setopt(handle, CURLOPT_UNRESTRICTED_AUTH, 1L));
}

SEXP R_new_handle(){
  reference *ref = calloc(1, sizeof(reference));
  ref->handle = curl_easy_init();
  set_handle_defaults(ref->handle);
  SEXP ptr = PROTECT(R_MakeExternalPtr(ref, R_NilValue, R_NilValue));
  R_RegisterCFinalizerEx(ptr, fin_handle, 1);
  setAttrib(ptr, R_ClassSymbol, mkString("curl_handle"));
  UNPROTECT(1);
  return ptr;
}

SEXP R_handle_reset(SEXP ptr){
  //reset all fields
  reference *ref = get_ref(ptr);
  set_form(ref, NULL);
  set_headers(ref, NULL);
  curl_easy_reset(ref->handle);

  //restore default settings
  set_handle_defaults(ref->handle);
  return ScalarLogical(1);
}

int opt_is_linked_list(int key) {
  // These four options need linked lists of various forms - determined
  // from inspection of curl.h
  return
    key == 10023 || // CURLOPT_HTTPHEADER
    key == 10024 || // CURLOPT_HTTPPOST
    key == 10070 || // CURLOPT_TELNETOPTIONS
    key == 10228;   // CURLOPT_PROXYHEADER
}

SEXP R_handle_setopt(SEXP ptr, SEXP keys, SEXP values){
  CURL *handle = get_handle(ptr);
  SEXP optnames = getAttrib(values, R_NamesSymbol);

  if(!isInteger(keys))
    error("keys` must be an integer");

  if(!isVector(values))
    error("`values` must be a list");

  for(int i = 0; i < length(keys); i++){
    int key = INTEGER(keys)[i];
    const char* optname = CHAR(STRING_ELT(optnames, i));
    SEXP val = VECTOR_ELT(values, i);
    if(val == R_NilValue){
      assert(curl_easy_setopt(handle, key, NULL));
    } else if (key == CURLOPT_PROGRESSFUNCTION) {
      if (TYPEOF(val) != CLOSXP)
        error("Value for option %s (%d) must be a function.", optname, key);

      assert(curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION,
        R_curl_callback_progress));
      assert(curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, val));
    } else if (key == CURLOPT_READFUNCTION) {
      if (TYPEOF(val) != CLOSXP)
        error("Value for option %s (%d) must be a function.", optname, key);

      assert(curl_easy_setopt(handle, CURLOPT_READFUNCTION,
        R_curl_callback_read));
      assert(curl_easy_setopt(handle, CURLOPT_READDATA, val));
    } else if (key == CURLOPT_DEBUGFUNCTION) {
      if (TYPEOF(val) != CLOSXP)
        error("Value for option %s (%d) must be a function.", optname, key);

      assert(curl_easy_setopt(handle, CURLOPT_DEBUGFUNCTION,
        R_curl_callback_debug));
      assert(curl_easy_setopt(handle, CURLOPT_DEBUGDATA, val));
    } else if (opt_is_linked_list(key)) {
      error("Option %s (%d) not supported.", optname, key);
    } else if(key < 10000){
      if(!isNumeric(val) || length(val) != 1) {
        error("Value for option %s (%d) must be a number.", optname, key);
      }
      assert(curl_easy_setopt(handle, key, (long) asInteger(val)));
    } else if(key < 20000){
      if(!isString(val) || length(val) != 1){
        error("Value for option %s (%d) must be a string.", optname, key);
      }
      assert(curl_easy_setopt(handle, key, CHAR(STRING_ELT(val, 0))));
    } else {
      error("Option %s (%d) not supported.", optname, key);
    }
  }
  return ScalarLogical(1);
}

SEXP R_handle_setheaders(SEXP ptr, SEXP vec){
  if(!isString(vec))
    error("header vector must be a string.");
  set_headers(get_ref(ptr), vec_to_slist(vec));
  return ScalarLogical(1);
}

SEXP R_handle_setform(SEXP ptr, SEXP form){
  if(!isVector(form))
    error("Form must be a list.");
  set_form(get_ref(ptr), make_form(form));
  return ScalarLogical(1);
}


