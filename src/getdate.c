#include <curl/curl.h>
#include <Rinternals.h>

SEXP R_curl_getdate(SEXP datestring) {
  if(!isString(datestring))
    error("Argument 'datestring' must be string.");

  /* convert date */
  time_t date = curl_getdate(CHAR(asChar(datestring)), NULL);

  if(date < 0)
    error("Failed to parse datestring.");

  return ScalarInteger((int) date);
}
