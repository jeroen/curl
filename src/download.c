/* *
 * Reimplementation of C_download (the "internal" method for download.file).
 * Because the user can interrupt the download with R_CheckUserInterrupt,
 * we need to do the cleanup in a separate function.
 */
#include <curl/curl.h>
#include <Rinternals.h>
#include "utils.h"

/* callback function to store received data */
static size_t push(void* contents, size_t sz, size_t nmemb, FILE *ctx) {
  if (pending_interrupt())
    return 0;
  return fwrite(contents, sz, nmemb, ctx);
}

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
  FILE *dest = fopen(translateCharUTF8(asChar(destfile)), CHAR(asChar(mode)));
  if(!dest)
    error("Failed to open file %s.", translateCharUTF8(asChar(destfile)));

  /* set options */
  curl_easy_setopt(handle, CURLOPT_URL, translateCharUTF8(asChar(url)));
  curl_easy_setopt(handle, CURLOPT_NOPROGRESS, asLogical(quiet));
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, push);
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
