#include "curl-common.h"

#if LIBCURL_VERSION_MAJOR > 7 || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR >= 46)
#define HAS_CURLFORM_CONTENTLEN 1
#endif


struct curl_httppost* make_form(SEXP form){
  struct curl_httppost* post = NULL;
  struct curl_httppost* last = NULL;
  SEXP ln = PROTECT(getAttrib(form, R_NamesSymbol));
  for(int i = 0; i < length(form); i++){
    const char *name = translateCharUTF8(STRING_ELT(ln, i));
    SEXP val = VECTOR_ELT(form, i);
    if(TYPEOF(val) == RAWSXP){
#if defined(HAS_CURLFORM_CONTENTLEN)
      curl_off_t datalen = Rf_xlength(val);
#else
      long datalen = Rf_length(val);
#endif
      if(datalen > 0){
        unsigned char * data = RAW(val);
#if defined(HAS_CURLFORM_CONTENTLEN)
        curl_formadd(&post, &last, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, data, CURLFORM_CONTENTLEN, datalen, CURLFORM_END);
#else
        curl_formadd(&post, &last, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, data, CURLFORM_CONTENTSLENGTH, datalen, CURLFORM_END);
#endif
      } else {
        //Note if 'CURLFORM_CONTENTLEN == 0' then libcurl assumes strlen() !
        curl_formadd(&post, &last, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, "", CURLFORM_END);
      }
    } else if(isVector(val) && Rf_length(val)){
      if(isString(VECTOR_ELT(val, 0))){
        //assume a form_file upload
        const char * path = CHAR(asChar(VECTOR_ELT(val, 0)));
        if(isString(VECTOR_ELT(val, 1))){
          const char *content_type = CHAR(asChar(VECTOR_ELT(val, 1)));
          curl_formadd(&post, &last, CURLFORM_COPYNAME, name, CURLFORM_FILE, path, CURLFORM_CONTENTTYPE, content_type, CURLFORM_END);
        } else {
          curl_formadd(&post, &last, CURLFORM_COPYNAME, name, CURLFORM_FILE, path, CURLFORM_END);
        }
      } else {
        //assume a form_value upload
        unsigned char * data = RAW(VECTOR_ELT(val, 0));
#if defined(HAS_CURLFORM_CONTENTLEN)
        curl_off_t datalen = Rf_xlength(VECTOR_ELT(val, 0));
#else
        long datalen = Rf_length(VECTOR_ELT(val, 0));
#endif
        if(isString(VECTOR_ELT(val, 1))){
          const char * content_type = CHAR(asChar(VECTOR_ELT(val, 1)));
#if defined(HAS_CURLFORM_CONTENTLEN)
          curl_formadd(&post, &last, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, data, CURLFORM_CONTENTLEN, datalen, CURLFORM_CONTENTTYPE, content_type, CURLFORM_END);
#else
          curl_formadd(&post, &last, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, data, CURLFORM_CONTENTSLENGTH, datalen, CURLFORM_CONTENTTYPE, content_type, CURLFORM_END);
#endif
        } else {
          curl_formadd(&post, &last, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, data, CURLFORM_CONTENTSLENGTH, datalen, CURLFORM_END);
        }
      }
    } else {
      error("form value %s not supported", name);
    }
  }
  UNPROTECT(1);
  return post;
}
