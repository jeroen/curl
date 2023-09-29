#include <Rinternals.h>


#ifdef _WIN32
#include <windows.h>
#include <sys/time.h>
static const char *lastError(){
  DWORD res = GetLastError();
  static char buf[1000], *p;
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, res,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                buf, 1000, NULL);
  p = buf+strlen(buf) -1;
  if(*p == '\n') *p = '\0';
  p = buf+strlen(buf) -1;
  if(*p == '\r') *p = '\0';
  p = buf+strlen(buf) -1;
  if(*p == '.') *p = '\0';
  return buf;
}
#else
#include <errno.h>
#include <string.h>
static const char *lastError(){
  return strerror(errno);
}
#endif

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
  size_t len = 0;
  if(Rf_length(buf)){
    if(fp == NULL){
      SEXP path = VECTOR_ELT(R_ExternalPtrTag(ptr), 0);
      SEXP append = VECTOR_ELT(R_ExternalPtrTag(ptr), 1);
      fp = fopen(CHAR(STRING_ELT(path, 0)), Rf_asLogical(append) ? "ab" : "wb");
      if(!fp)
        Rf_error("Failed to open file: %s\n%s", CHAR(STRING_ELT(path, 0)), lastError());
      R_SetExternalPtrAddr(ptr, fp);
      total_open_writers++;
    }
    len = fwrite(RAW(buf), 1, Rf_xlength(buf), fp);
  }
  if(Rf_asLogical(close)){
    fin_file_writer(ptr);
  } else if(Rf_length(buf)) {
    fflush(fp);
  }
  return ScalarInteger(len);
}

SEXP R_new_file_writer(SEXP opts){
  SEXP ptr = PROTECT(R_MakeExternalPtr(NULL, opts, R_NilValue));
  R_RegisterCFinalizerEx(ptr, fin_file_writer, TRUE);
  setAttrib(ptr, R_ClassSymbol, mkString("file_writer"));
  UNPROTECT(1);
  return ptr;
}

SEXP R_total_writers(void){
  return(ScalarInteger(total_open_writers));
}
