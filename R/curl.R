#' Curl connection
#'
#' Drop-in replacement for url() that uses libcurl to support https.
#'
#' @useDynLib curl R_curl_connection
#' @export
#' @examples test <- curl()
#' open(test)
#' close(test)
curl <- function(url = "http://httpbin.org/get"){
  .Call(R_curl_connection, url)
}