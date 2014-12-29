#include <curl/curl.h>
#include <Rinternals.h>
#include "options.h"

SEXP R_curl_options() {
  /* Count number options extracted from curl.h */
  int len = sizeof(curl_options) / sizeof(curl_options[0]);

  /* Create R vectors */
  SEXP names = PROTECT(allocVector(STRSXP, len));
  SEXP values = PROTECT(allocVector(INTSXP, len));
  for(int i = 0; i < len; i++) {
    SET_STRING_ELT(names, i, mkChar(curl_options[i].name));
    INTEGER(values)[i] = curl_options[i].val;
  }
  setAttrib(values, R_NamesSymbol, names);
  UNPROTECT(2);
  return values;
}
