#include <curl/curl.h>
#include <Rinternals.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

struct curl_httppost* make_form(SEXP form){
  struct curl_httppost* post = NULL;
  struct curl_httppost* last = NULL;
  SEXP ln = getAttrib(form, R_NamesSymbol);
  for(int i = 0; i < length(form); i++){
    const char *name = translateCharUTF8(STRING_ELT(ln, i));
    SEXP val = VECTOR_ELT(form, i);
    if(isString(val)){
      curl_formadd(&post, &last, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, translateCharUTF8(asChar(val)), CURLFORM_END);
    } else if(TYPEOF(val) == RAWSXP){
      unsigned char *buf = malloc(length(val));
      memcpy(buf, RAW(val), length(val));
      curl_formadd(&post, &last, CURLFORM_COPYNAME, name, CURLFORM_PTRCONTENTS, buf, CURLFORM_CONTENTSLENGTH, length(val), CURLFORM_END);
    } else{
      error("form value %s not supported", name);
    }
  }
  return post;
}
