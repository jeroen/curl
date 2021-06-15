#include "curl-common.h"

#include <R_ext/libextern.h>    // R_interrupts_suspended +

LibExtern Rboolean R_interrupts_suspended;
LibExtern int R_interrupts_pending;

#ifdef _WIN32
#include <Rembedded.h>
SEXP R_interrupt() {
  UserBreak = 1;
  R_CheckUserInterrupt();
  return R_NilValue;
}
#else
#include <Rinterface.h>
SEXP R_interrupt() {
  Rf_onintr();
  return R_NilValue;
}
#endif

int R_curl_callback_progress(SEXP fun,
                             double dltotal, double dlnow,
                             double ultotal, double ulnow) {

  SEXP down = PROTECT(allocVector(REALSXP, 2));
  REAL(down)[0] = dltotal;
  REAL(down)[1] = dlnow;

  SEXP up = PROTECT(allocVector(REALSXP, 2));
  REAL(up)[0] = ultotal;
  REAL(up)[1] = ulnow;

  SEXP ints = PROTECT(Rf_ScalarInteger(R_interrupts_suspended));
  SEXP call = PROTECT(Rf_lang5(curl_safe_eval, fun, down, up, ints));
  int ok;
  R_interrupts_suspended = TRUE;
  if (curl_last_error != NULL) {
    R_ReleaseObject(curl_last_error);
    curl_last_error = NULL;
  }
  SEXP res = PROTECT(R_tryEval(call, R_GlobalEnv, &ok));

  if (ok != 0) {
    UNPROTECT(5);
    return CURL_READFUNC_ABORT;
  }

  if (!isNull(VECTOR_ELT(res, 1))) {
    curl_last_error = VECTOR_ELT(res, 1);
    R_PreserveObject(curl_last_error);
    UNPROTECT(5);
    return CURL_READFUNC_ABORT;
  }

  SEXP resok = VECTOR_ELT(res, 0);
  if (TYPEOF(resok) != LGLSXP || length(resok) != 1) {
    UNPROTECT(5);
    Rf_warning("progress callback must return boolean");
    return 0;
  }

  int out = asLogical(resok);
  UNPROTECT(5);
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
