#include "curl-common.h"

SEXP R_option_types(){
#ifdef CURLOT_FLAG_ALIAS
  int len = 0;
  const struct curl_easyoption *o = curl_easy_option_next(NULL);
  while(o){
    len++;
    o = curl_easy_option_next(o);
  }
  SEXP names = Rf_allocVector(STRSXP, len);
  SEXP values = Rf_allocVector(INTSXP, len);
  SEXP types = Rf_allocVector(INTSXP, len);
  SEXP alias = Rf_allocVector(LGLSXP, len);
  for(int i = 0; i < len; i++){
    o = curl_easy_option_next(o);
    SET_STRING_ELT(names, i, Rf_mkChar(o->name ? o->name : "???"));
    INTEGER(values)[i] = o->id;
    INTEGER(types)[i] = o->type;
    LOGICAL(alias)[i] = o->flags & CURLOT_FLAG_ALIAS;
  }

  SEXP out = PROTECT(Rf_allocVector(VECSXP, 4));
  SEXP listnms = PROTECT(Rf_allocVector(STRSXP, 4));
  Rf_setAttrib(out, R_NamesSymbol, listnms);
  SET_VECTOR_ELT(out, 0, names);
  SET_VECTOR_ELT(out, 1, values);
  SET_VECTOR_ELT(out, 2, types);
  SET_VECTOR_ELT(out, 3, alias);
  SET_STRING_ELT(listnms, 0, Rf_mkChar("name"));
  SET_STRING_ELT(listnms, 1, Rf_mkChar("value"));
  SET_STRING_ELT(listnms, 2, Rf_mkChar("type"));
  SET_STRING_ELT(listnms, 3, Rf_mkChar("alias"));
  UNPROTECT(2);
  return out;
#else
  return R_NilValue;
#endif
}
