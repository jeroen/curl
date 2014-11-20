/* *
 * Reimplementation of C_download (the "internal" method for download.file).
 */
#include <curl/curl.h>
#include <curl/easy.h>
#include <Rinternals.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

SEXP R_download_curl(SEXP url, SEXP destfile, SEXP quiet, SEXP mode) {
  if(!isString(url))
    error("Argument 'url' must be string.");

  if(!isString(destfile))
    error("Argument 'destfile' must be string.");

  if(!isLogical(quiet))
    error("Argument 'quiet' must be TRUE/FALSE.");

  if(!isString(mode))
    error("Argument 'mode' must be string.");

  /* Open file descriptor */
  FILE *dest = fopen(translateCharUTF8(asChar(destfile)), CHAR(asChar(mode)));

  /* setup curl */
  CURL *http_handle = make_handle(translateCharUTF8(asChar(url)));
  curl_easy_setopt(http_handle, CURLOPT_WRITEDATA, dest);
  curl_easy_setopt(http_handle, CURLOPT_NOPROGRESS, asLogical(quiet));

  /* perform request */
  CURLcode res = curl_easy_perform(http_handle);
  fclose(dest);
  assert(res);
  stop_for_status(http_handle);

  return ScalarInteger(0);
}


