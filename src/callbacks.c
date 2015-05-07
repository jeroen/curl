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

size_t R_curl_callback_read(char *buffer, size_t size, size_t nitems, SEXP fun) {
  SEXP nbytes = PROTECT(ScalarInteger(size * nitems));
  SEXP call = PROTECT(LCONS(fun, LCONS(nbytes, R_NilValue)));

  int ok;
  SEXP res = PROTECT(R_tryEval(call, R_GlobalEnv, &ok));

  if (ok != 0 || pending_interrupt()) {
    UNPROTECT(3);
    return CURL_READFUNC_ABORT;
  }

  if (TYPEOF(res) != RAWSXP) {
    UNPROTECT(3);
    Rf_warning("progress callback must return boolean");
    return CURL_READFUNC_ABORT;
  }

  size_t bytes_read = length(res);
  memcpy(buffer, RAW(res), bytes_read);

  UNPROTECT(3);
  return bytes_read;
}

int R_curl_callback_debug(CURL *handle, curl_infotype type_, char *data,
                          size_t size, SEXP fun) {

  SEXP type = PROTECT(ScalarInteger(type_));

  SEXP msg = PROTECT(allocVector(STRSXP, 1));
  SET_STRING_ELT(msg, 0, Rf_mkCharLen(data, size));

  SEXP call = PROTECT(LCONS(fun, LCONS(type, LCONS(msg, R_NilValue))));

  int ok;
  R_tryEval(call, R_GlobalEnv, &ok);

  UNPROTECT(3);
  return 0;
}

