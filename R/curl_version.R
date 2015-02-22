#' @rdname utilities
#' @useDynLib curl R_curl_version
#' @export
#' @examples # Curl/ssl version info
#' curl_version()
curl_version <- function(){
  .Call(R_curl_version);
}

#' @rdname utilities
#' @useDynLib curl R_curl_options
#' @export
#' @examples # List available curl options
#' curl_options()
curl_options <- function(){
  .Call(R_curl_options);
}

#' @rdname utilities
#' @param datestring a string with a timestamp
#' @useDynLib curl R_curl_getdate
#' @export
#' @examples
#' # Parse dates in many formats
#' curl_getdate("Sunday, 06-Nov-94 08:49:37 GMT")
#' curl_getdate("06 Nov 1994 08:49:37")
#' curl_getdate("20040911 +0200")
curl_getdate <- function(datestring){
  out <- .Call(R_curl_getdate, datestring);
  class(out) <- c("POSIXct", "POSIXt")
  out
}
