#include <Rinternals.h>

static int total_open_writers = 0;

void fin_file_writer(SEXP ptr){
  FILE *fp = R_ExternalPtrAddr(ptr);
  if(fp != NULL){
    fclose(fp);
    R_ClearExternalPtr(ptr);
    total_open_writers--;
  }
}

SEXP R_write_file_writer(SEXP ptr, SEXP buf, SEXP close){
  FILE *fp = R_ExternalPtrAddr(ptr);
  if(fp == NULL){
    SEXP path = R_ExternalPtrTag(ptr);
    fp = fopen(CHAR(STRING_ELT(path, 0)), "wb");
    if(!fp)
      Rf_error("Failed to open file: %s", CHAR(STRING_ELT(path, 0)));
    R_SetExternalPtrAddr(ptr, fp);
    total_open_writers++;
  }
  size_t len = fwrite(RAW(buf), 1, Rf_length(buf), fp);
  if(Rf_asLogical(close)){
    fin_file_writer(ptr);
  } else if(Rf_length(buf)) {
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

SEXP R_total_writers(){
  return(ScalarInteger(total_open_writers));
}
