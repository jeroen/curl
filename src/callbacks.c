#include "curl-common.h"

int R_curl_callback_progress(SEXP fun,
                             double dltotal, double dlnow,
                             double ultotal, double ulnow) {

  SEXP down = PROTECT(allocVector(REALSXP, 2));
  REAL(down)[0] = dltotal;
  REAL(down)[1] = dlnow;

  SEXP up = PROTECT(allocVector(REALSXP, 2));
  REAL(up)[0] = ultotal;
  REAL(up)[1] = ulnow;

  SEXP call = PROTECT(Rf_lang3(fun, down, up));
  int ok;
  SEXP res = PROTECT(R_tryEval(call, R_GlobalEnv, &ok));

  if (ok != 0) {
    UNPROTECT(4);
    return CURL_READFUNC_ABORT;
  }

  if (TYPEOF(res) != LGLSXP || length(res) != 1) {
    UNPROTECT(4);
    Rf_warning("progress callback must return boolean");
    return 0;
  }

  int out = asLogical(res);
  UNPROTECT(4);
  return !out;
}

size_t R_curl_callback_read(char *buffer, size_t size, size_t nitems, SEXP fun) {
  SEXP nbytes = PROTECT(ScalarInteger(size * nitems));
  SEXP call = PROTECT(Rf_lang2(fun, nbytes));

  int ok;
  SEXP res = PROTECT(R_tryEval(call, R_GlobalEnv, &ok));

  if (ok != 0) {
    UNPROTECT(3);
    return CURL_READFUNC_ABORT;
  }

  if (TYPEOF(res) != RAWSXP) {
    UNPROTECT(3);
    Rf_warning("read callback must raw vector");
    return CURL_READFUNC_ABORT;
  }

  size_t bytes_read = length(res);
  memcpy(buffer, RAW(res), bytes_read);

  UNPROTECT(3);
  return bytes_read;
}

int R_curl_callback_debug(CURL *handle, curl_infotype type_, char *data,
                          size_t size, SEXP fun) {

  /* wrap type and msg into R types */
  SEXP type = PROTECT(ScalarInteger(type_));
  SEXP msg = PROTECT(allocVector(RAWSXP, size));
  memcpy(RAW(msg), data, size);

  /* call the R function */
  SEXP call = PROTECT(Rf_lang3(fun, type, msg));
  R_tryEval(call, R_GlobalEnv, NULL);

  UNPROTECT(3);
  // Debug function must always return 0
  return 0;
}


int R_curl_callback_xferinfo(SEXP fun,
                             curl_off_t  dltotal, curl_off_t  dlnow,
                             curl_off_t  ultotal, curl_off_t  ulnow) {
  return R_curl_callback_progress(fun, dltotal, dlnow, ultotal, ulnow);
}
