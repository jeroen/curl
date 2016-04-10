#include <curl/curl.h>
#include <Rinternals.h>

#define make_string(x) x ? Rf_mkString(x) : ScalarString(NA_STRING)

SEXP R_curl_version() {
  /* retrieve info from curl */
  const curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);

  /* put stuff in a list */
  SEXP list = PROTECT(allocVector(VECSXP, 7));
  SET_VECTOR_ELT(list, 0, make_string(data->version));
  SET_VECTOR_ELT(list, 1, make_string(data->ssl_version));
  SET_VECTOR_ELT(list, 2, make_string(data->libz_version));
  SET_VECTOR_ELT(list, 3, make_string(data->libssh_version));
  SET_VECTOR_ELT(list, 4, make_string(data->libidn));
  SET_VECTOR_ELT(list, 5, make_string(data->host));

  /* create vector of protocols */
  const char *const * temp = data->protocols;
  while(*temp) temp++;
  int len = temp - data->protocols;
  SEXP protocols = PROTECT(allocVector(STRSXP, len));
  for (int i = 0; i < len; i++){
    SET_STRING_ELT(protocols, i, mkChar(*(data->protocols + i)));
  }
  SET_VECTOR_ELT(list, 6, protocols);

  /* add list names */
  SEXP names = PROTECT(allocVector(STRSXP, 7));
  SET_STRING_ELT(names, 0, mkChar("version"));
  SET_STRING_ELT(names, 1, mkChar("ssl_version"));
  SET_STRING_ELT(names, 2, mkChar("libz_version"));
  SET_STRING_ELT(names, 3, mkChar("libssh_version"));
  SET_STRING_ELT(names, 4, mkChar("libidn_version"));
  SET_STRING_ELT(names, 5, mkChar("host"));
  SET_STRING_ELT(names, 6, mkChar("protocols"));
  setAttrib(list, R_NamesSymbol, names);

  /* return */
  UNPROTECT(3);
  return list;
}
