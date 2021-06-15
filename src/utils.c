#include "curl-common.h"
#include <stdint.h> /* SIZE_MAX */

#include <R_ext/libextern.h>    // R_interrupts_suspended +

LibExtern Rboolean R_interrupts_suspended;

SEXP curl_safe_eval = NULL;
SEXP curl_last_error = NULL;

SEXP R_set_r_callbacks(SEXP eval) {
  curl_safe_eval = eval;
  R_PreserveObject(eval);
  return R_NilValue;
}

SEXP R_enable_interrupts() {
  int old = R_interrupts_suspended;
  R_interrupts_suspended = FALSE;
  return ScalarInteger(old);
}

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

static char * parse_host(const char * input){
  static char buf[8000] = {0};
  char *url = buf;
  strncpy(url, input, 7999);

  char *ptr = NULL;
  if((ptr = strstr(url, "://")))
    url = ptr + 3;
  if((ptr = strchr(url, '/')))
    *ptr = 0;
  if((ptr = strchr(url, '#')))
    *ptr = 0;
  if((ptr = strchr(url, '?')))
    *ptr = 0;
  if((ptr = strchr(url, '@')))
    url = ptr + 1;
  return url;
}

static R_INLINE SEXP new_env(SEXP parent) {
  SEXP env;
  PROTECT(env = allocSExp(ENVSXP));
  SET_FRAME(env, R_NilValue);
  SET_ENCLOS(env, parent);
  SET_HASHTAB(env, R_NilValue);
  SET_ATTRIB(env, R_NilValue);
  UNPROTECT(1);
  return env;
}

void curl_throw_error(SEXP err) {
  SEXP env = PROTECT(new_env(R_BaseEnv));
  Rf_defineVar(Rf_install("err"), err, env);
  SEXP call = PROTECT(Rf_lang2(Rf_install("stop"), err));
  Rf_eval(call, env);
}

void assert_status(CURLcode res, reference *ref){
  // Customize better error message for timeoutsS
  if(res == CURLE_OPERATION_TIMEDOUT){
    const char *url = NULL;
    if(curl_easy_getinfo(ref->handle, CURLINFO_EFFECTIVE_URL, &url) == CURLE_OK){
      Rf_error("%s: [%s] %s", curl_easy_strerror(res), parse_host(url), ref->errbuf);
    }
  }
  if(res != CURLE_OK) {
    if(curl_last_error != NULL) {
      SEXP err = PROTECT(curl_last_error);
      R_ReleaseObject(curl_last_error);
      curl_last_error = NULL;
      curl_throw_error(err);
    } else {
      Rf_error("%s", strlen(ref->errbuf) ? ref->errbuf : curl_easy_strerror(res));
    }
  }
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

static size_t round_up(size_t v){
  if(v == 0)
    return 0;
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
/* 64 bit only */
#if SIZE_MAX > 4294967296
    v |= v >> 32;
#endif
  return ++v;
}

size_t append_buffer(void *contents, size_t sz, size_t nmemb, void *ctx) {
//if (pending_interrupt())
  //  return 0;

  /* avoids compiler warning on windows */
  size_t realsize = sz * nmemb;
  memory *mem = (memory*) ctx;

  /* realloc can be slow, therefore increase buffer to nearest 2^n */
  mem->buf = realloc(mem->buf, round_up(mem->size + realsize));
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
