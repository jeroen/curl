#include <curl/curl.h>
#include <Rinternals.h>
#include "utils.h"

CURL* get_handle(SEXP ptr){
  if(!R_ExternalPtrAddr(ptr))
    error("handle is dead");
  reference *ref = (reference*) R_ExternalPtrAddr(ptr);
  return ref->handle;
}

reference* get_ref(SEXP ptr){
  if(!R_ExternalPtrAddr(ptr))
    error("handle is dead");
  reference *ref = (reference*) R_ExternalPtrAddr(ptr);
  return ref;
}

void set_form(reference *ref, struct curl_httppost* newform){
  if(ref->form)
    curl_formfree(ref->form);
  ref->form = newform;
  assert(curl_easy_setopt(ref->handle, CURLOPT_HTTPPOST, ref->form));
}

void set_headers(reference *ref, struct curl_slist *newheaders){
  if(ref->headers)
    curl_slist_free_all(ref->headers);
  ref->headers = newheaders;
  assert(curl_easy_setopt(ref->handle, CURLOPT_HTTPHEADER, ref->headers));
}

void assert(CURLcode res){
  if(res != CURLE_OK)
    error(curl_easy_strerror(res));
}

void stop_for_status(CURL *http_handle){
  long status = 0;
  assert(curl_easy_getinfo(http_handle, CURLINFO_RESPONSE_CODE, &status));

  /* check http status code. Not sure what this does for ftp. */
  if(status >= 300)
    error("HTTP error %d.", status);
}

/* make sure to call curl_slist_free_all on this object */
struct curl_slist* vec_to_slist(SEXP vec){
  if(!isString(vec))
    error("vec is not a character vector");
  struct curl_slist *slist = NULL;
  for(int i = 0; i < length(vec); i++){
    slist = curl_slist_append(slist, CHAR(STRING_ELT(vec, i)));
  }
  return slist;
}

SEXP slist_to_vec(struct curl_slist *slist){
  /* linked list of strings */
  struct curl_slist *cursor = slist;

  /* count slist */
  int n = 0;
  while (cursor) {
    n++;
    cursor = cursor->next;
  }

  SEXP out = PROTECT(allocVector(STRSXP, n));
  cursor = slist;
  for(int i = 0; i < n; i++){
    SET_STRING_ELT(out, i, mkChar(cursor->data));
    cursor = cursor->next;
  }
  UNPROTECT(1);
  return out;
}

SEXP R_global_cleanup() {
  curl_global_cleanup();
  return R_NilValue;
}
