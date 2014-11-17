#' Curl connection
#'
#' Drop-in replacement for url() that uses libcurl to support https.
#'
#' @useDynLib curl R_curl_connection
#' @export
#' @examples test <- curl()
#' open(test)
#' readLines(test)
#' close(test)
#'
#' # HTTP error
#' curl("https://httpbin.org/status/418", "r")
#'
#' # HTTP redirects
#' readLines(curl("https://httpbin.org/redirect/3"))
#'
#' # Binary data
#' buf <- readBin(curl("http://httpbin.org/bytes/12345", "rb"), raw(), 99999)
#' length(buf)
#'
#' # Error after redirect
#' curl("https://httpbin.org/redirect-to?url=http://httpbin.org/status/418", "r")
#'
#' # FTP support
#' read.csv(curl("ftp://cran.r-project.org/pub/R/CRAN_mirrors.csv"))
#'
#'  # Stream large dataset over https with gzip
#' library(jsonlite)
#' con <- gzcon(curl("https://jeroenooms.github.io/data/nycflights13.json.gz"))
#' nycflights <- stream_in(con)
#'
#' # Accept-Encoding: compress, gzip
#' readLines(curl("http://httpbin.org/gzip))
#' readLines(curl("http://httpbin.org/deflate))
#'
curl <- function(url = "http://httpbin.org/get", open = ""){
  .Call(R_curl_connection, url, open)
}