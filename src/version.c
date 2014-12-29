#include <curl/curl.h>
#include <Rinternals.h>

SEXP R_curl_version() {
  /* retrieve info from curl */
  const curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);

  /* put stuff in a list */
  SEXP list = PROTECT(allocVector(VECSXP, 5));
  SET_VECTOR_ELT(list, 0, mkString(data->version));
  SET_VECTOR_ELT(list, 1, mkString(data->ssl_version));
  SET_VECTOR_ELT(list, 2, mkString(data->libz_version));
  SET_VECTOR_ELT(list, 3, mkString(data->host));

  /* create vector of protocols */
  const char *const * temp = data->protocols;
  while(*temp) temp++;
  int len = temp - data->protocols;
  SEXP protocols = PROTECT(allocVector(STRSXP, len));
  for (int i = 0; i < len; i++){
    SET_STRING_ELT(protocols, i, mkChar(*(data->protocols + i)));
  }
  SET_VECTOR_ELT(list, 4, protocols);

  /* add list names */
  SEXP names = PROTECT(allocVector(STRSXP, 5));
  SET_STRING_ELT(names, 0, mkChar("version"));
  SET_STRING_ELT(names, 1, mkChar("ssl_version"));
  SET_STRING_ELT(names, 2, mkChar("libz_version"));
  SET_STRING_ELT(names, 3, mkChar("host"));
  SET_STRING_ELT(names, 4, mkChar("protocols"));
  setAttrib(list, R_NamesSymbol, names);

  /* return */
  UNPROTECT(3);
  return list;
}
