#include "curl-common.h"

CURL* get_handle(SEXP ptr){
  return get_ref(ptr)->handle;
}

reference* get_ref(SEXP ptr){
  if(TYPEOF(ptr) != EXTPTRSXP || !Rf_inherits(ptr, "curl_handle"))
    Rf_error("handle is not a curl_handle()");
  if(!R_ExternalPtrAddr(ptr))
    error("handle is dead");
  reference *ref = (reference*) R_ExternalPtrAddr(ptr);
  return ref;
}

void set_form(reference *ref, struct curl_httppost* newform){
  if(ref->form)
    curl_formfree(ref->form);
  ref->form = newform;
  if(newform){
    assert(curl_easy_setopt(ref->handle, CURLOPT_HTTPPOST, ref->form));
  } else {
    //CURLOPT_HTTPPOST has bug for empty forms. We probably want this:
    assert(curl_easy_setopt(ref->handle, CURLOPT_POSTFIELDS, ""));
  }
}

void set_headers(reference *ref, struct curl_slist *newheaders){
  if(ref->headers)
    curl_slist_free_all(ref->headers);
  ref->headers = newheaders;
  assert(curl_easy_setopt(ref->handle, CURLOPT_HTTPHEADER, ref->headers));
}

void reset_resheaders(reference *ref){
  if(ref->resheaders.buf)
    free(ref->resheaders.buf);
  ref->resheaders.buf = NULL;
  ref->resheaders.size = 0;
}

void reset_errbuf(reference *ref){
  memset(ref->errbuf, 0, CURL_ERROR_SIZE);
}

void assert(CURLcode res){
  if(res != CURLE_OK)
    error(curl_easy_strerror(res));
}

void assert_status(CURLcode res, reference *ref){
  if(res == CURLE_OPERATION_TIMEDOUT)
    Rf_error("%s: %s", curl_easy_strerror(res), ref->errbuf);
  if(res != CURLE_OK)
    Rf_error("%s", strlen(ref->errbuf) ? ref->errbuf : curl_easy_strerror(res));
}

void massert(CURLMcode res){
  if(res != CURLM_OK)
    error(curl_multi_strerror(res));
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

size_t push_disk(void* contents, size_t sz, size_t nmemb, FILE *ctx) {
  //if (pending_interrupt())
  //  return 0;
  return fwrite(contents, sz, nmemb, ctx);
}

size_t append_buffer(void *contents, size_t sz, size_t nmemb, void *ctx) {
//if (pending_interrupt())
  //  return 0;

  /* avoids compiler warning on windows */
  size_t realsize = sz * nmemb;
  memory *mem = (memory*) ctx;

  /* realloc is slow on windows, therefore increase buffer to nearest 2^n */
  #ifdef _WIN32
    mem->buf = realloc(mem->buf, exp2(ceil(log2(mem->size + realsize))));
  #else
    mem->buf = realloc(mem->buf, mem->size + realsize);
  #endif

  if (!mem->buf)
    return 0;

  /* append data and increment size */
  memcpy(&(mem->buf[mem->size]), contents, realsize);
  mem->size += realsize;
  return realsize;
}

size_t data_callback(void * data, size_t sz, size_t nmemb, SEXP fun) {
  size_t size = sz * nmemb;
  SEXP buf = PROTECT(allocVector(RAWSXP, size));
  memcpy(RAW(buf), data, size);

  /* call the R function */
  int err;
  SEXP call = PROTECT(Rf_lang3(fun, buf, ScalarInteger(0)));
  R_tryEval(call, R_GlobalEnv, &err);
  UNPROTECT(2);
  return err ? 0 : size;
}
