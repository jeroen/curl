/* *
 * This is where the actual HTTP request settings are made.
 */
#include <curl/curl.h>
#include <curl/easy.h>
#include <Rinternals.h>
#include <string.h>
#include <stdlib.h>

CURL *make_handle(const char *url){
  /* construct new handler */
  CURL *http_handle = curl_easy_init();

  /* curl configuration options */
  curl_easy_setopt(http_handle, CURLOPT_URL, url);

  //#ifdef _WIN32
  curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYPEER, 0L);
  //#endif

  curl_easy_setopt(http_handle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(http_handle, CURLOPT_CONNECTTIMEOUT_MS, 10*1000);

  /* aka 'CURLOPT_ACCEPT_ENCODING' in recent versions */
  curl_easy_setopt(http_handle, CURLOPT_ENCODING, "gzip, deflate");

  /* set http request headers */
  struct curl_slist *reqheaders = NULL;
  reqheaders = curl_slist_append(reqheaders, "User-Agent: r/curl/jeroen");
  reqheaders = curl_slist_append(reqheaders, "Accept-Charset: utf-8");
  reqheaders = curl_slist_append(reqheaders, "Cache-Control: no-cache");
  curl_easy_setopt(http_handle, CURLOPT_HTTPHEADER, reqheaders);

  /*return the handler */
  return http_handle;
}

void assert(CURLcode res){
  if(res != CURLE_OK)
    error(curl_easy_strerror(res));
}

void stop_for_status(CURL *http_handle){
  long status = 0;
  assert(curl_easy_getinfo(http_handle, CURLINFO_RESPONSE_CODE, &status));

  /* check http status code. Not sure what this does for ftp. */
  if(status >= 300)
    error("HTTP error %d.", status);
}

SEXP R_global_cleanup() {
  curl_global_cleanup();
  return R_NilValue;
}

