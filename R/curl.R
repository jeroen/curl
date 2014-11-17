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
#' curl("http://httpbin.org/status/418", "r")
#'
#' # http redirects
#' curl("http://httpbin.org/redirect/3", "r")
#'
#' # redirect to error
#' curl("http://httpbin.org/redirect-to?url=http://httpbin.org/status/418", "r")
#'
#' # works for ftp too
#' curl("ftp://cran.r-project.org/pub/R/CRAN_mirrors.csv", "r")
#'
#' # stream over https with gzip
#' library(jsonlite)
#' con <- gzcon(curl("https://jeroenooms.github.io/data/nycflights13.json.gz", "rb"))
#' test <- stream_in(con)
curl <- function(url = "http://httpbin.org/get", mode = ""){
  .Call(R_curl_connection, url, mode)
}