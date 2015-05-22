/* *
 * Blocking easy interfaces to libcurl for R.
 * Example: http://curl.haxx.se/libcurl/c/getinmemory.html
 */

#include "curl-common.h"

SEXP R_curl_fetch_memory(SEXP url, SEXP ptr){
  if (!isString(url) || length(url) != 1)
    error("Argument 'url' must be string.");

  /* get the handle */
  CURL *handle = get_handle(ptr);

  /* update the url */
  curl_easy_setopt(handle, CURLOPT_URL, translateCharUTF8(asChar(url)));

  /* reset the response header buffer */
  reset_resheaders(get_ref(ptr));

  /* buffer body */
  memory body = {NULL, 0};
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, append_buffer);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, &body);

  /* perform blocking request */
  CURLcode status = curl_easy_perform(handle);

  /* Reset for reuse */
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, NULL);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, NULL);

  /* check for errors */
  if (status != CURLE_OK) {
    free(body.buf);
    error(curl_easy_strerror(status));
  }

  /* create output */
  SEXP out = PROTECT(allocVector(RAWSXP, body.size));

  /* copy only if there is actual content */
  if(body.size)
    memcpy(RAW(out), body.buf, body.size);

  /* cleanup and return */
  UNPROTECT(1);
  free(body.buf);
  return out;
}

SEXP R_curl_fetch_disk(SEXP url, SEXP ptr, SEXP path, SEXP mode){
  if (!isString(url) || length(url) != 1)
    error("Argument 'url' must be string.");
  if (!isString(path) || length(path) != 1)
    error("`path` must be string.");

  /* get the handle */
  CURL *handle = get_handle(ptr);

  /* update the url */
  curl_easy_setopt(handle, CURLOPT_URL, translateCharUTF8(asChar(url)));

  /* reset the response header buffer */
  reset_resheaders(get_ref(ptr));

  /* open file */
  FILE *dest = fopen(translateCharUTF8(asChar(path)), CHAR(asChar(mode)));
  if(!dest)
    error("Failed to open file %s.", translateCharUTF8(asChar(path)));
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, push_disk);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, dest);

  /* perform blocking request */
  CURLcode status = curl_easy_perform(handle);

  /* cleanup */
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, NULL);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, NULL);
  fclose(dest);

  /* check for errors */
  if (status != CURLE_OK) {
    error(curl_easy_strerror(status));
  }

  /* return the file path */
  return path;
}
