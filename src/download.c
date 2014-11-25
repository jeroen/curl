/* *
 * Reimplementation of C_download (the "internal" method for download.file).
 * Because the user can interrupt the download with R_CheckUserInterrupt,
 * we need to do the cleanup in a separate function.
 */
#include <curl/curl.h>
#include <curl/easy.h>
#include <Rinternals.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

FILE *dest;
CURL *handle;

/* callback function to store received data */
static size_t push(void *contents, size_t sz, size_t nmemb, void *ctx) {
  R_CheckUserInterrupt();
  return fwrite(contents, sz, nmemb, ctx);
}

SEXP R_download_curl(SEXP url, SEXP destfile, SEXP quiet, SEXP mode) {
  if(!isString(url))
    error("Argument 'url' must be string.");

  if(!isString(destfile))
    error("Argument 'destfile' must be string.");

  if(!isLogical(quiet))
    error("Argument 'quiet' must be TRUE/FALSE.");

  if(!isString(mode))
    error("Argument 'mode' must be string.");

  /* init curl */
  handle = make_handle(translateCharUTF8(asChar(url)));
  curl_easy_setopt(handle, CURLOPT_NOPROGRESS, asLogical(quiet));

  /* open file */
  dest = fopen(translateCharUTF8(asChar(destfile)), CHAR(asChar(mode)));
  if(!dest)
    error("Failed to open file %s.", translateCharUTF8(asChar(destfile)));
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, dest);

  /* Custom writefun only to call R_CheckUserInterrupt */
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, push);

  /* perform blocking request */
  CURLcode success = curl_easy_perform(handle);
  assert(success);
  stop_for_status(handle);
  return ScalarInteger(0);
}

SEXP R_download_cleanup(){
  if(dest) {
    fclose(dest);
    dest = NULL;
  }
  if(handle){
    curl_easy_cleanup(handle);
    handle = NULL;
  }
  return R_NilValue;
}
