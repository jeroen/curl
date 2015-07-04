#include <curl/curl.h>
#include <Rinternals.h>

SEXP R_curl_escape(SEXP url, SEXP unescape_) {
  if (TYPEOF(url) != STRSXP)
    error("`url` must be a character vector.");

  /* init curl */
  CURL *curl = curl_easy_init();
  if (!curl)
    return(R_NilValue);

  int unescape = asLogical(unescape_);
  int n = Rf_length(url);
  SEXP output = PROTECT(allocVector(STRSXP, n));

  for (int i = 0; i < n; ++i) {
    const char *in = CHAR(STRING_ELT(url, i));
    char *out;
    if (unescape) {
      out = curl_easy_unescape(curl, in, 0, NULL);
    } else {
      out = curl_easy_escape(curl, in, 0);
    }

    SET_STRING_ELT(output, i, mkCharCE(out, CE_UTF8));
    curl_free(out);
  }

  curl_easy_cleanup(curl);
  UNPROTECT(1);
  return output;
}
