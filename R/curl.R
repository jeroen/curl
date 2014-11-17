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
#' curl("https://httpbin.org/status/418", "r")
#'
#' # http redirects
#' readLines(curl("https://httpbin.org/redirect/3"))
#'
#' # redirect to error
#' curl("https://httpbin.org/redirect-to?url=http://httpbin.org/status/418", "r")
#'
#' # works for ftp too
#' read.csv(curl("ftp://cran.r-project.org/pub/R/CRAN_mirrors.csv"))
#'
#' # stream over https with gzip
#' library(jsonlite)
#' con <- gzcon(curl("https://jeroenooms.github.io/data/nycflights13.json.gz"))
#' test <- stream_in(con)
curl <- function(url = "http://httpbin.org/get", mode = ""){
  .Call(R_curl_connection, url, mode)
}