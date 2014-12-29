#' Curl utility functions
#'
#' Utility functions for working with curl.
#'
#' @useDynLib curl R_curl_escape
#' @export
#' @name utility functions
#' @rdname utilities
#' @param url a string (typically a url or parameter) to be URL encoded
#' @examples curl_version()
#' out <- curl_escape("foo = bar + 5")
#' curl_unescape(out)
curl_escape <- function(url){
  .Call(R_curl_escape, url, FALSE);
}

#' @rdname utilities
#' @export
curl_unescape <- function(url){
  .Call(R_curl_escape, url, TRUE);
}
