#' Fetch the contents of a URL
#'
#' Low-level bindings to write data from a URL into memory, disk or a callback
#' function. These are mainly intended for \code{httr}, most users will be better
#' off using the \code{\link{curl}} or \code{\link{curl_download}} function, or the
#' http specific wrappers in the \code{httr} package.
#'
#' The curl_fetch functions automatically raise an error upon protocol problems
#' (network, disk, ssl) but do not implement application logic. For example for
#' you need to check the status code of http requests yourself in the response,
#' and deal with it accordingly.
#'
#' Both \code{curl_fetch_memory} and \code{curl_fetch_disk} have a blocking and
#' non-blocking C implementation. The latter is slightly slower but allows for
#' interrupting the download prematurely (using e.g. CTRL+C or ESC). Interrupting
#' is enabled when R runs in interactive mode or when
#' \code{getOption("curl_interrupt") == TRUE}.
#'
#' @param url A character string naming the URL of a resource to be downloaded.
#' @param handle a curl handle object
#' @export
#' @rdname curl_fetch
#' @useDynLib curl R_curl_fetch_memory
#' @examples
#' # Load in memory
#' res <- curl_fetch_memory("http://httpbin.org/cookies/set?foo=123&bar=ftw")
#' res$content
#'
#' # Save to disk
#' res <- curl_fetch_disk("http://httpbin.org/stream/10", tempfile())
#' res$content
#' readLines(res$content)
#'
#' # Stream with callback
#' res <- curl_fetch_stream("http://httpbin.org/stream/20", function(x){
#'   cat(rawToChar(x))
#' })
curl_fetch_memory <- function(url, handle = new_handle()){
  nonblocking <- isTRUE(getOption("curl_interrupt", interactive()))
  output <- .Call(R_curl_fetch_memory, url, handle, nonblocking)
  res <- handle_response_data(handle)
  res$content <- output
  res
}

#' @export
#' @param path Path to save results
#' @rdname curl_fetch
#' @useDynLib curl R_curl_fetch_disk
curl_fetch_disk <- function(url, path, handle = new_handle()){
  nonblocking <- isTRUE(getOption("curl_interrupt", interactive()))
  path <- normalizePath(path, mustWork = FALSE)
  output <- .Call(R_curl_fetch_disk, url, handle, path, "wb", nonblocking)
  res <- handle_response_data(handle)
  res$content <- output
  res
}

#' @export
#' @param fun Callback function. Should have one argument, which will be
#'   a raw vector.
#' @rdname curl_fetch
#' @useDynLib curl R_curl_connection
curl_fetch_stream <- function(url, fun, handle = new_handle()){
  con <- .Call(R_curl_connection, url, "rb", handle, FALSE)
  on.exit(close(con))
  while(length(bin <- readBin(con, raw(), 8192L))){
    fun(bin)
  }
  handle_response_data(handle)
}
