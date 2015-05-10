#include "curl-common.h"

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
      curl_formadd(&post, &last, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, RAW(val), CURLFORM_CONTENTSLENGTH, (long) length(val), CURLFORM_END);
    } else if(isVector(val)){
      //assume a form_upload value
      const char* path = translateCharUTF8(asChar(VECTOR_ELT(val, 0)));
      if(VECTOR_ELT(val, 1) == R_NilValue){
        curl_formadd(&post, &last, CURLFORM_COPYNAME, name, CURLFORM_FILE, path, CURLFORM_END);
      } else {
        const char *content_type = translateCharUTF8(asChar(VECTOR_ELT(val, 1)));
        curl_formadd(&post, &last, CURLFORM_COPYNAME, name, CURLFORM_FILE, path, CURLFORM_CONTENTTYPE, content_type, CURLFORM_END);
      }
    } else {
      error("form value %s not supported", name);
    }
  }
  return post;
}
