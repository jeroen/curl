#' Curl connection
#'
#' Drop-in replacement for url() that uses libcurl to support https.
#'
#' @useDynLib curl R_curl_connection
#' @export
#' @examples test <- curl()
#' open(test)
#' close(test)
#'
#' # http error
#' open(curl("http://httpbin.org/status/418"))
#'
#' # http redirects
#' open(curl("http://httpbin.org/redirect/3"))
#'
#' # redirect to error
#' open(curl("http://httpbin.org/redirect-to?url=http://httpbin.org/status/418"))
#'
#' # stream over https with gzip
#' library(jsonlite)
#' con <- gzcon(curl("https://jeroenooms.github.io/data/nycflights13.json.gz"))
#' test <- stream_in(con)
curl <- function(url = "http://httpbin.org/get"){
  .Call(R_curl_connection, url)
}