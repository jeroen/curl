#include <Rinternals.h>
#include <string.h>
#include <curl/curl.h>
#include "utils.h"

int eval_callback_bool(SEXP call) {
  int ok;
  SEXP res = PROTECT(R_tryEval(call, R_GlobalEnv, &ok));

  if (ok != 0 || pending_interrupt()) {
    UNPROTECT(1);
    return 0;
  }

  if (TYPEOF(res) != LGLSXP || length(res) != 1) {
    UNPROTECT(1);
    Rf_warning("progress callback must return boolean");
    return 0;
  }

  UNPROTECT(1);
  return asLogical(res);
}

int eval_callback_int(SEXP call) {
  int ok;
  SEXP res = PROTECT(R_tryEval(call, R_GlobalEnv, &ok));

  if (ok != 0 || pending_interrupt()) {
    UNPROTECT(1);
    return 0;
  }

  if (TYPEOF(res) != INTSXP || length(res) != 1) {
    UNPROTECT(1);
    Rf_warning("progress callback must return integer");
    return 0;
  }

  UNPROTECT(1);
  return asInteger(res);
}

int R_curl_callback_progress(SEXP fun,
                             double dltotal, double dlnow,
                             double ultotal, double ulnow) {

  SEXP down = PROTECT(allocVector(REALSXP, 2));
  REAL(down)[0] = dltotal;
  REAL(down)[1] = dlnow;

  SEXP up = PROTECT(allocVector(REALSXP, 2));
  REAL(up)[0] = ultotal;
  REAL(up)[1] = ulnow;

  SEXP call = PROTECT(LCONS(fun, LCONS(down, LCONS(up, R_NilValue))));
  int ok = eval_callback_bool(call);
  UNPROTECT(3);

  return !ok;
}

size_t R_curl_callback_write(void *buffer, size_t size, size_t nmemb,
                             SEXP fun) {
  SEXP data = PROTECT(allocVector(RAWSXP, size * nmemb));
  memcpy(RAW(data), buffer, size * nmemb);

  SEXP call = PROTECT(LCONS(fun, LCONS(data, R_NilValue)));
  int ok = eval_callback_int(call);
  UNPROTECT(2);

  return ok;
}

