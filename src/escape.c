#include <curl/curl.h>
#include <Rinternals.h>

SEXP R_curl_escape(SEXP url, SEXP unescape) {
  if(!isString(url))
    error("Argument 'url' must be string.");

  /* init curl */
  SEXP res;
  char *output;
  CURL *curl = curl_easy_init();
  if(curl) {
    if(asLogical(unescape)){
      output = curl_easy_unescape(curl, translateCharUTF8(asChar(url)), 0, NULL);
    } else {
      output = curl_easy_escape(curl, translateCharUTF8(asChar(url)), 0);
    }
    res = PROTECT(mkString(output));
    curl_free(output);
  } else {
    res = PROTECT(R_NilValue);
  }
  curl_easy_cleanup(curl);
  UNPROTECT(1);
  return res;
}

