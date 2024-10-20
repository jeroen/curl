#include "curl-common.h"

#ifdef HAS_CURL_PARSER
static void fail_if(CURLUcode err){
  if(err != CURLUE_OK)
#ifdef HAS_CURL_PARSER_STRERROR
    Rf_error("Failed to parse URL: %s", curl_url_strerror(err));
#else
    Rf_error("Failed to parse URL: error code %d", err);
#endif
}

static SEXP get_field(CURLU *h, CURLUPart part, CURLUcode field_missing){
  char *str = NULL;
  SEXP field = NULL;
  CURLUcode err = curl_url_get(h, part, &str, 0);
  if(err == field_missing && err != CURLUE_OK){
    field = NA_STRING;
  } else {
    fail_if(err);
    field = Rf_mkCharCE(str, CE_UTF8);
  }
  curl_free(str);
  return field;
}

static SEXP make_url_names(void){
  SEXP names = PROTECT(Rf_allocVector(STRSXP, 10));
  SET_STRING_ELT(names, 0, Rf_mkChar("url"));
  SET_STRING_ELT(names, 1, Rf_mkChar("scheme"));
  SET_STRING_ELT(names, 2, Rf_mkChar("host"));
  SET_STRING_ELT(names, 3, Rf_mkChar("port"));
  SET_STRING_ELT(names, 4, Rf_mkChar("path"));
  SET_STRING_ELT(names, 5, Rf_mkChar("query"));
  SET_STRING_ELT(names, 6, Rf_mkChar("fragment"));
  SET_STRING_ELT(names, 7, Rf_mkChar("user"));
  SET_STRING_ELT(names, 8, Rf_mkChar("password"));
  SET_STRING_ELT(names, 9, Rf_mkChar("options"));
  UNPROTECT(1);
  return names;
}

SEXP R_parse_url(SEXP url) {
  CURLU *h = curl_url();
  fail_if(curl_url_set(h, CURLUPART_URL, CHAR(STRING_ELT(url, 0)), 0));
  SEXP out = PROTECT(Rf_allocVector(STRSXP, 10));
  SET_STRING_ELT(out, 0, get_field(h, CURLUPART_URL, CURLUE_OK));
  SET_STRING_ELT(out, 1, get_field(h, CURLUPART_SCHEME, CURLUE_NO_SCHEME));
  SET_STRING_ELT(out, 2, get_field(h, CURLUPART_HOST, CURLUE_NO_HOST));
  SET_STRING_ELT(out, 3, get_field(h, CURLUPART_PORT, CURLUE_NO_PORT));
  SET_STRING_ELT(out, 4, get_field(h, CURLUPART_PATH, CURLUE_OK));
  SET_STRING_ELT(out, 5, get_field(h, CURLUPART_QUERY, CURLUE_NO_QUERY));
  SET_STRING_ELT(out, 6, get_field(h, CURLUPART_FRAGMENT, CURLUE_NO_FRAGMENT));
  SET_STRING_ELT(out, 7, get_field(h, CURLUPART_USER, CURLUE_NO_USER));
  SET_STRING_ELT(out, 8, get_field(h, CURLUPART_PASSWORD, CURLUE_NO_PASSWORD));
  SET_STRING_ELT(out, 9, get_field(h, CURLUPART_OPTIONS, CURLUE_NO_OPTIONS));
  curl_url_cleanup(h);
  Rf_setAttrib(out, R_NamesSymbol, make_url_names());
  UNPROTECT(1);
  return out;
}
#else
SEXP R_parse_url(SEXP url) {
  Rf_error("URL parser not suppored, this libcurl is too old");
}
#endif
