#' Curl connection interface
#'
#' Drop-in replacement for base \code{\link{url}} that supports https, ftps,
#' gzip, deflate, etc. Default behavior is identical to \code{\link{url}}, but
#' request can be fully configured by passing a custom \code{\link{handle}}.
#'
#' @useDynLib curl R_curl_connection
#' @export
#' @param url character string. See examples.
#' @param open character string. How to open the connection if it should be opened
#'   initially. Currently only "r" and "rb" are supported.
#' @param handle a curl handle object
#' @examples \dontrun{
#' con <- curl("https://httpbin.org/get")
#' readLines(con)
#'
#' # Auto-opened connections can be recycled
#' open(con, "rb")
#' bin <- readBin(con, raw(), 999)
#' close(con)
#' rawToChar(bin)
#'
#' # HTTP error
#' curl("https://httpbin.org/status/418", "r")
#'
#' # Follow redirects
#' readLines(curl("https://httpbin.org/redirect/3"))
#'
#' # Error after redirect
#' curl("https://httpbin.org/redirect-to?url=http://httpbin.org/status/418", "r")
#'
#' # Auto decompress Accept-Encoding: gzip / deflate (rfc2616 #14.3)
#' readLines(curl("http://httpbin.org/gzip"))
#' readLines(curl("http://httpbin.org/deflate"))
#'
#' # Binary support
#' buf <- readBin(curl("http://httpbin.org/bytes/98765", "rb"), raw(), 1e5)
#' length(buf)
#'
#' # Read file from disk
#' test <- paste0("file://", system.file("DESCRIPTION"))
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
curl <- function(url = "http://httpbin.org/get", open = "", handle = new_handle()){
  .Call(R_curl_connection, url, open, handle, TRUE)
}
