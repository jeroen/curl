#include <Rinternals.h>

void fin_file_writer(SEXP ptr){
  FILE *fp = R_ExternalPtrAddr(ptr);
  if(fp != NULL){
    fclose(fp);
    R_ClearExternalPtr(ptr);
  }
}

SEXP R_write_file_writer(SEXP ptr, SEXP buf, SEXP close){
  FILE *fp = R_ExternalPtrAddr(ptr);
  if(fp == NULL){
    SEXP path = R_ExternalPtrTag(ptr);
    fp = fopen(CHAR(STRING_ELT(path, 0)), "wb");
    R_SetExternalPtrAddr(ptr, fp);
  }
  size_t len = fwrite(RAW(buf), 1, Rf_length(buf), fp);
  if(Rf_asInteger(close)){
    fin_file_writer(ptr);
  } else {
    fflush(fp);
  }
  return ScalarInteger(len);
}

SEXP R_new_file_writer(SEXP path){
  SEXP ptr = PROTECT(R_MakeExternalPtr(NULL, path, R_NilValue));
  R_RegisterCFinalizerEx(ptr, fin_file_writer, TRUE);
  setAttrib(ptr, R_ClassSymbol, mkString("file_writer"));
  UNPROTECT(1);
  return ptr;
}
