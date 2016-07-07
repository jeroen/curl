#include "curl-common.h"
#include "callbacks.h"

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

char CA_BUNDLE[MAX_PATH];

SEXP R_set_bundle(SEXP path){
  strcpy(CA_BUNDLE, CHAR(asChar(path)));
  return mkString(CA_BUNDLE);
}

SEXP R_get_bundle(){
  return mkString(CA_BUNDLE);
}

void clean_handle(reference *ref){
  if(ref->refCount == 0){
    //Rprintf("cleaning easy handle\n");
    if(ref->headers)
      curl_slist_free_all(ref->headers);
    if(ref->form)
      curl_formfree(ref->form);
    if(ref->handle)
      curl_easy_cleanup(ref->handle);
    if(ref->resheaders.buf)
      free(ref->resheaders.buf);
    free(ref);
  }
}

void fin_handle(SEXP ptr){
  //Rprintf("running finalizer...\n");
  reference *ref = (reference*) R_ExternalPtrAddr(ptr);
  ref->refCount--;
  clean_handle(ref);
  R_ClearExternalPtr(ptr);
}

/* These are defaulst that we always want to set */
void set_handle_defaults(reference *ref){

  /* the actual curl handle */
  CURL *handle = ref->handle;
  assert(curl_easy_setopt(handle, CURLOPT_PRIVATE, ref));

  /* set the response header collector */
  reset_resheaders(ref);
  curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, append_buffer);
  curl_easy_setopt(handle, CURLOPT_HEADERDATA, &(ref->resheaders));

  #ifdef _WIN32
  if(CA_BUNDLE != NULL && strlen(CA_BUNDLE)){
    /* on windows a cert bundle is included with R version 3.2.0 */
    curl_easy_setopt(handle, CURLOPT_CAINFO, CA_BUNDLE);
    //Rprintf("Using bundle %s\n", CA_BUNDLE);
  } else {
    /* disable cert validation for older versions of R */
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
  }
  #endif

  /* needed to support compressed responses */
  assert(curl_easy_setopt(handle, CURLOPT_ENCODING, "gzip, deflate"));

  /* follow redirect */
  assert(curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L));
  assert(curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 10L));

  /* a sensible timeout (10s) */
  assert(curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 10L));

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
  ref->refCount = 1;
  ref->handle = curl_easy_init();
  set_handle_defaults(ref);
  SEXP ptr = PROTECT(R_MakeExternalPtr(ref, R_NilValue, R_NilValue));
  R_RegisterCFinalizerEx(ptr, fin_handle, 1);
  setAttrib(ptr, R_ClassSymbol, mkString("curl_handle"));
  UNPROTECT(1);
  ref->handleptr = ptr;
  return ptr;
}

SEXP R_handle_reset(SEXP ptr){
  //reset all fields
  reference *ref = get_ref(ptr);
  set_form(ref, NULL);
  set_headers(ref, NULL);
  curl_easy_reset(ref->handle);

  //restore default settings
  set_handle_defaults(ref);
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
        (curl_progress_callback) R_curl_callback_progress));
      assert(curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, val));
    } else if (key == CURLOPT_READFUNCTION) {
      if (TYPEOF(val) != CLOSXP)
        error("Value for option %s (%d) must be a function.", optname, key);

      assert(curl_easy_setopt(handle, CURLOPT_READFUNCTION,
        (curl_read_callback) R_curl_callback_read));
      assert(curl_easy_setopt(handle, CURLOPT_READDATA, val));
    } else if (key == CURLOPT_DEBUGFUNCTION) {
      if (TYPEOF(val) != CLOSXP)
        error("Value for option %s (%d) must be a function.", optname, key);

      assert(curl_easy_setopt(handle, CURLOPT_DEBUGFUNCTION,
        (curl_debug_callback) R_curl_callback_debug));
      assert(curl_easy_setopt(handle, CURLOPT_DEBUGDATA, val));
    } else if (opt_is_linked_list(key)) {
      error("Option %s (%d) not supported.", optname, key);
    } else if(key < 10000){
      if(!isNumeric(val) || length(val) != 1) {
        error("Value for option %s (%d) must be a number.", optname, key);
      }
      assert(curl_easy_setopt(handle, key, (long) asInteger(val)));
    } else if(key < 20000){
      switch (TYPEOF(val)) {
      case RAWSXP:
        assert(curl_easy_setopt(handle, key, RAW(val)));
        break;
      case STRSXP:
        if (length(val) != 1)
          error("Value for option %s (%d) must be length-1 string", optname, key);
        assert(curl_easy_setopt(handle, key, CHAR(STRING_ELT(val, 0))));
        break;
      default:
        error("Value for option %s (%d) must be a string or raw vector.", optname, key);
      }
    } else if(key >= 30000 && key < 40000){
      if(!isNumeric(val) || length(val) != 1) {
        error("Value for option %s (%d) must be a number.", optname, key);
      }
      assert(curl_easy_setopt(handle, key, (curl_off_t) asReal(val)));
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
  SET_STRING_ELT(names, 3, mkChar("modified"));
  SET_STRING_ELT(names, 4, mkChar("times"));
  SET_STRING_ELT(names, 5, mkChar("content"));
  UNPROTECT(1);
  return names;
}

SEXP R_get_handle_cookies(SEXP ptr){
  return make_cookievec(get_handle(ptr));
}

SEXP make_handle_response(reference *ref){
  CURL *handle = ref->handle;
  SEXP res = PROTECT(allocVector(VECSXP, 6));
  SET_VECTOR_ELT(res, 0, make_url(handle));
  SET_VECTOR_ELT(res, 1, make_status(handle));
  SET_VECTOR_ELT(res, 2, make_rawvec(ref->resheaders.buf, ref->resheaders.size));
  SET_VECTOR_ELT(res, 3, make_filetime(handle));
  SET_VECTOR_ELT(res, 4, make_timevec(handle));
  SET_VECTOR_ELT(res, 5, R_NilValue);
  setAttrib(res, R_NamesSymbol, make_namesvec());
  UNPROTECT(1);
  return res;
}

SEXP R_get_handle_response(SEXP ptr){
  /* get the handle */
  reference *ref = get_ref(ptr);
  return make_handle_response(ref);
}
