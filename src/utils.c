#include "curl-common.h"
#include <ctype.h>

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

void assert_status(CURLcode res, reference *ref){
  if(res == CURLE_OPERATION_TIMEDOUT) {
    // ref->errbuf contains "Resolving timed out after 1 milliseconds" in this case.
    // This branch adds the host to the error message to aid folk looking at logs, #190
    const char *url=NULL;
    curl_easy_getinfo(ref->handle, CURLINFO_EFFECTIVE_URL, &url);
    if (url && (
        strncmp(url, "http://", 7)==0 ||
        strncmp(url, "HTTP://", 7)==0 ||
        strncmp(url, "https://", 8)==0 ||
        strncmp(url, "HTTPS://", 8)==0 ||
        strncmp(url, "ftp://", 6)==0 ||
        strncmp(url, "FTP://", 6)==0)) {
      // only attempt to extract hostname from this strict subset of known schemes for
      // safety to avoid leaking non-hostname data to error message and log
      url = strchr(url, ':') + 3;  // we're sure by now that that :// exists
      // *url now points to the first character after "scheme://"
      // now, as first step, chop off the URL from the first of any '/', '?', or '#'
      // probably the first '/' already removes any ? or # afterwards, but do them too anyway
      const char *w = strchr(url, '/');
      int len = w ? w-url : strlen(url);  // exclude from the first '/', if any
      w = memchr(url, '?', len);
      if (w) len = w-url;                 // exclude from the first '?' (if any) reducing len
      w = memchr(url, '#', len);
      if (w) len = w-url;                 // exclude from the first '#' (if any) reducing len
      // Now find the first '@' if any and jump over it, to exclude username:password, just in case curl_easy_getinfo did not already exclude that
      w = memchr(url, '@', len);
      if (w) { len-=(w-url+1); url=w+1; }
      // *url now points to the hostname of length len, possibly including a :port
      // now check the hostname is valid containing only expected characters, and only print it if so
      if (len>0 && (isalnum(url[0]))) { // hostnames must start with a-zA-Z0-9 (not . or -)
        int i=0;
        while (i<len && (isalnum(url[i]) || url[i]=='.' || url[i]=='-')) i++;  // hostname valid characters
        if (i<len-1 && url[i]==':') {  // len-1 because there must at least one digit after the : (if any) otherwise invalid
          i++;  // skip over :
          while (i<len && isdigit(url[i])) i++;  // port number must consist of 0-9 only
        }
        if (i==len) { // "hostname" or "hostname:port" is valid; all len characters have been checked
          Rf_error("%s: %s: %.*s", curl_easy_strerror(res), ref->errbuf, len, url);
        }
      }
    }
    // fallback to a very plain simple error which does not use *url at all.
    Rf_error("%s: %s: (hostname could not be safely extracted from URL)", curl_easy_strerror(res), ref->errbuf);
  }
  if(res != CURLE_OK) {
    // in cases other than CURLE_OPERATION_TIMEDOUT, curl's own message includes the hostname already
    // e.g. res==7 (CURLE_COULDNT_CONNECT), ref->errbuf contains "Failed to connect to www.google.com port 81: Connection timed out"
    Rf_error("%s", strlen(ref->errbuf) ? ref->errbuf : curl_easy_strerror(res));
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
  if (sizeof(size_t) == 8)
    v |= v >> 32;
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
