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

  /* open file */
  FILE *dest = fopen(translateCharUTF8(asChar(destfile)), CHAR(asChar(mode)));
  if(!dest)
    error("Failed to open file %s.", translateCharUTF8(asChar(destfile)));

  /* init curl */
  CURL *req = make_handle(translateCharUTF8(asChar(url)));
  curl_easy_setopt(req, CURLOPT_NOPROGRESS, asLogical(quiet));
  curl_easy_setopt(req, CURLOPT_WRITEDATA, dest);
  CURLcode success = curl_easy_perform(req);

  /* post processing */
  fclose(dest);
  assert(success);
  stop_for_status(req);
  curl_easy_cleanup(req);
  return ScalarInteger(0);
}
