#' Curl connection
#'
#' Drop-in replacement for url() that uses libcurl. Supports http(s), ftp(s), file,
#' gzip, deflate, etc.
#'
#' @useDynLib curl R_curl_connection
#' @export
#' @param url character string. See examples.
#' @param open character string. How to open the connection if it should be opened
#'   initially. Currently only "r" and "rb" are supported.
#' @examples test <- curl("http://httpbin.org/get")
#' open(test)
#' readLines(test, warn = FALSE)
#' close(test)
#'
#' \dontrun{
#' # HTTP error
#' curl("https://httpbin.org/status/418", "r")
#'
#' # HTTP redirects
#' readLines(curl("https://httpbin.org/redirect/3"), warn = FALSE)
#'
#' # Binary data
#' buf <- readBin(curl("http://httpbin.org/bytes/12345", "rb"), raw(), 99999)
#' length(buf)
#'
#' # Error after redirect
#' curl("https://httpbin.org/redirect-to?url=http://httpbin.org/status/418", "r")
#'
#' # Accept-Encoding: compress, gzip
#' readLines(curl("http://httpbin.org/gzip"), warn = FALSE)
#' readLines(curl("http://httpbin.org/deflate"), warn = FALSE)
#'
#' # File support
#' test <- paste0("file://", system.file("CITATION"))
#' readLines(curl(test))
#'
#' # Other protocols
#' read.csv(curl("ftp://cran.r-project.org/pub/R/CRAN_mirrors.csv"))
#' readLines(curl("ftps://test.rebex.net:990/readme.txt"))
#' readLines(curl("gopher://quux.org/1"))
#'
#' # Streaming data
#' con <- curl("http://jeroenooms.github.io/data/diamonds.json", "r")
#' while(length(x <- readLines(con, n = 5))){
#'   print(x)
#' }
#'
#' # Stream large dataset over https with gzip
#' library(jsonlite)
#' con <- gzcon(curl("https://jeroenooms.github.io/data/nycflights13.json.gz"))
#' nycflights <- stream_in(con)
#' }
#'
curl <- function(url = "http://httpbin.org/get", open = ""){
  .Call(R_curl_connection, url, open)
}