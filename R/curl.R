#' Curl connection interface
#'
#' Drop-in replacement for base [url()] that supports https, ftps,
#' gzip, deflate, etc. Default behavior is identical to [url()], but
#' request can be fully configured by passing a custom [handle()].
#'
#' As of version 2.3 curl connections support `open(con, blocking = FALSE)`.
#' In this case `readBin` and `readLines` will return immediately with data
#' that is available without waiting. For such non-blocking connections the caller
#' needs to call [isIncomplete()] to check if the download has completed
#' yet.
#'
#' Requesting a connection in read-write mode (`"r+"` or `"r+b"`) creates a
#' bidirectional socket connection, similar to base [socketConnection()]. In
#' this mode libcurl only establishes the connection (including TLS handshake,
#' certificate verification, and proxy traversal, cf. `CURLOPT_CONNECT_ONLY`)
#' without speaking the protocol, and you can use e.g. [writeLines()] and
#' [readLines()] to implement a custom protocol dialogue over the socket.
#' This can be used to speak custom protocols over TLS, or protocol
#' extensions that libcurl does not implement, such as IMAP `IDLE`.
#'
#' Unlike regular curl connections, socket connections are not opened
#' automatically: call [open()] on the connection yourself. Blocking reads
#' wait for data to arrive (interruptible via ctrl-c), which is convenient
#' for a request/response dialogue, but note that reading until EOF, such as
#' `readLines(con)` without `n`, waits indefinitely because a socket has no
#' EOF until the peer closes. To poll without waiting, open the connection
#' with `open(con, blocking = FALSE)`: reads then return only the data that
#' is available, and [isIncomplete()] tells whether the peer is still
#' connected. Note that for pingpong protocols such as `imaps://` or
#' `smtps://`, libcurl consumes the server greeting while connecting, so you
#' should not expect to read it. A socket connection cannot be recycled for
#' regular transfers.
#'
#' @useDynLib curl R_curl_connection
#' @export
#' @param url character string. See examples.
#' @param open character string. How to open the connection if it should be opened
#'   initially. Use "r" or "rb" for a regular transfer, or "r+" / "r+b" for a
#'   bidirectional (connect-only) socket connection, see details. Socket
#'   connections are never opened initially: call [open()] on the connection
#'   yourself, optionally with `blocking = FALSE`.
#' @param handle a curl handle object
#' @examples \dontrun{
#' con <- curl("https://hb.cran.dev/get")
#' readLines(con)
#'
#' # Auto-opened connections can be recycled
#' open(con, "rb")
#' bin <- readBin(con, raw(), 999)
#' close(con)
#' rawToChar(bin)
#'
#' # HTTP error
#' curl("https://hb.cran.dev/status/418", "r")
#'
#' # Follow redirects
#' readLines(curl("https://hb.cran.dev/redirect/3"))
#'
#' # Error after redirect
#' curl("https://hb.cran.dev/redirect-to?url=https://hb.cran.dev/status/418", "r")
#'
#' # Auto decompress Accept-Encoding: gzip / deflate (rfc2616 #14.3)
#' readLines(curl("https://hb.cran.dev/gzip"))
#' readLines(curl("https://hb.cran.dev/deflate"))
#'
#' # Binary support
#' buf <- readBin(curl("https://hb.cran.dev/bytes/98765", "rb"), raw(), 1e5)
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
#' con <- curl("http://jeroen.github.io/data/diamonds.json", "r")
#' while(length(x <- readLines(con, n = 5))){
#'   print(x)
#' }
#'
#' # Stream large dataset over https with gzip
#' library(jsonlite)
#' con <- gzcon(curl("https://jeroen.github.io/data/nycflights13.json.gz"))
#' nycflights <- stream_in(con)
#'
#' # Raw socket connection (CURLOPT_CONNECT_ONLY): speak IMAP by hand over TLS
#' con <- curl("imaps://imap.gmail.com", "r+")
#' open(con)
#' writeLines("A1 CAPABILITY", con, sep = "\r\n")
#' while(!grepl("^A1 ", line <- readLines(con, n = 1))) print(line)
#' writeLines("A2 LOGOUT", con, sep = "\r\n")
#' close(con)
#' }
#'
curl <- function(url = "https://hb.cran.dev/get", open = "", handle = new_handle()){
  curl_connection(url, open, handle)
}

# 'stream' currently only used for non-blocking connections to prevent
# busy looping in curl_fetch_stream()
curl_connection <- function(url, mode, handle, partial = FALSE){
  socket <- grepl("+", mode, fixed = TRUE)
  con <- .Call(R_curl_connection, url, handle, partial, socket)
  if(!identical(mode, "") && !socket){
    withCallingHandlers(open(con, open = mode), error = function(err) {
      close(con)
    })
  }
  return(con)
}
