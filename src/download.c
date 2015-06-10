/* *
 * Reimplementation of C_download (the "internal" method for download.file).
 */
#include "curl-common.h"

SEXP R_download_curl(SEXP url, SEXP destfile, SEXP quiet, SEXP mode, SEXP ptr) {
  if(!isString(url))
    error("Argument 'url' must be string.");

  if(!isString(destfile))
    error("Argument 'destfile' must be string.");

  if(!isLogical(quiet))
    error("Argument 'quiet' must be TRUE/FALSE.");

  if(!isString(mode))
    error("Argument 'mode' must be string.");

  /* get the handle */
  CURL *handle = get_handle(ptr);

  /* open file */
  FILE *dest = fopen(CHAR(asChar(destfile)), CHAR(asChar(mode)));
  if(!dest)
    error("Failed to open file %s.", CHAR(asChar(destfile)));

  /* set options */
  curl_easy_setopt(handle, CURLOPT_URL, translateCharUTF8(asChar(url)));
  curl_easy_setopt(handle, CURLOPT_NOPROGRESS, asLogical(quiet));
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, push_disk);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, dest);

  /* perform blocking request */
  CURLcode status = curl_easy_perform(handle);

  /* cleanup */
  curl_easy_setopt(handle, CURLOPT_URL, NULL);
  curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, NULL);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, NULL);
  fclose(dest);

  if (status != CURLE_OK)
    error(curl_easy_strerror(status));

  /* check for success */
  stop_for_status(handle);
  return ScalarInteger(0);
}
