#include "curl-common.h"
#include "callbacks.h"

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#if LIBCURL_VERSION_MAJOR > 7 || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR >= 47)
#define HAS_HTTP_VERSION_2TLS 1
#endif

#if LIBCURL_VERSION_MAJOR > 7 || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR >= 32)
#define HAS_XFERINFOFUNCTION 1
#endif

#if LIBCURL_VERSION_MAJOR > 7 || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR >= 36)
#define HAS_CURLOPT_EXPECT_100_TIMEOUT_MS 1
#endif


char CA_BUNDLE[MAX_PATH];
static struct curl_slist * default_headers;

SEXP R_set_bundle(SEXP path){
  strcpy(CA_BUNDLE, CHAR(asChar(path)));
  return mkString(CA_BUNDLE);
}

SEXP R_get_bundle(){
  return mkString(CA_BUNDLE);
}

int total_handles = 0;

void clean_handle(reference *ref){
  if(ref->refCount == 0){
    if(ref->headers)
      curl_slist_free_all(ref->headers);
    if(ref->form)
      curl_formfree(ref->form);
    if(ref->handle)
      curl_easy_cleanup(ref->handle);
    if(ref->resheaders.buf)
      free(ref->resheaders.buf);
    free(ref);
    total_handles--;
  }
}

void fin_handle(SEXP ptr){
  reference *ref = (reference*) R_ExternalPtrAddr(ptr);

  //this kind of strange but the multi finalizer needs the ptr value
  //if it is still pending
  ref->refCount--;
  if(ref->refCount == 0)
    R_ClearExternalPtr(ptr);

  //free stuff
  clean_handle(ref);
}

/* the default readfunc os fread which can cause R to freeze */
size_t dummy_read(char *buffer, size_t size, size_t nitems, void *instream){
  return 0;
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

  /* set the default user agent */
  SEXP agent = GetOption1(install("HTTPUserAgent"));
  if(isString(agent) && Rf_length(agent)){
    assert(curl_easy_setopt(handle, CURLOPT_USERAGENT, CHAR(STRING_ELT(agent, 0))));
  } else {
    assert(curl_easy_setopt(handle, CURLOPT_USERAGENT, "r/curl/jeroen"));
  }

  /* allow all authentication methods */
  assert(curl_easy_setopt(handle, CURLOPT_HTTPAUTH, CURLAUTH_ANY));
  assert(curl_easy_setopt(handle, CURLOPT_UNRESTRICTED_AUTH, 1L));
  assert(curl_easy_setopt(handle, CURLOPT_PROXYAUTH, CURLAUTH_ANY));

  /* enables HTTP2 on HTTPS (match behavior of curl cmd util) */
#if defined(CURL_VERSION_HTTP2) && defined(HAS_HTTP_VERSION_2TLS)
  if(curl_version_info(CURLVERSION_NOW)->features & CURL_VERSION_HTTP2)
    assert(curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS));
#endif

  /* set an error buffer */
  assert(curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, ref->errbuf));

  /* dummy readfunction because default can freeze R */
  assert(curl_easy_setopt(handle, CURLOPT_READFUNCTION, dummy_read));

  /* seems to be needed for native WinSSL */
#ifdef _WIN32
  curl_easy_setopt(handle, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);
#endif

  /* set default headers (disables the Expect: http 100)*/
#ifdef HAS_CURLOPT_EXPECT_100_TIMEOUT_MS
  assert(curl_easy_setopt(handle, CURLOPT_EXPECT_100_TIMEOUT_MS, 0L));
#endif
  assert(curl_easy_setopt(handle, CURLOPT_HTTPHEADER, default_headers));
}

SEXP R_new_handle(){
  reference *ref = calloc(1, sizeof(reference));
  ref->refCount = 1;
  ref->handle = curl_easy_init();
  total_handles++;
  set_handle_defaults(ref);
  SEXP ptr = PROTECT(R_MakeExternalPtr(ref, R_NilValue, R_NilValue));
  R_RegisterCFinalizerEx(ptr, fin_handle, TRUE);
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
  reset_errbuf(ref);
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
  SEXP optnames = PROTECT(getAttrib(values, R_NamesSymbol));

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
#ifdef HAS_XFERINFOFUNCTION
    } else if (key == CURLOPT_XFERINFOFUNCTION) {
      if (TYPEOF(val) != CLOSXP)
        error("Value for option %s (%d) must be a function.", optname, key);

      assert(curl_easy_setopt(handle, CURLOPT_XFERINFOFUNCTION,
                              (curl_progress_callback) R_curl_callback_xferinfo));
      assert(curl_easy_setopt(handle, CURLOPT_XFERINFODATA, val));
      assert(curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0));
#endif
    } else if (key == CURLOPT_PROGRESSFUNCTION) {
      if (TYPEOF(val) != CLOSXP)
        error("Value for option %s (%d) must be a function.", optname, key);

      assert(curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION,
        (curl_progress_callback) R_curl_callback_progress));
      assert(curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, val));
      assert(curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0));
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
    } else if (key == CURLOPT_URL) {
      /* always use utf-8 for urls */
      const char * url_utf8 = translateCharUTF8(STRING_ELT(val, 0));
      assert(curl_easy_setopt(handle, CURLOPT_URL, url_utf8));
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
        if(key == CURLOPT_POSTFIELDS || key == CURLOPT_COPYPOSTFIELDS)
          assert(curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t) Rf_length(val)));
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
  UNPROTECT(1);
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
  return ScalarString(mkCharCE(res_url, CE_UTF8));
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
  if(size > 0)
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

SEXP R_total_handles(){
  return(ScalarInteger(total_handles));
}
