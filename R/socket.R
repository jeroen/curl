#' Raw socket access on a connect-only handle
#'
#' \code{curl_connect()} lets libcurl establish a connection (TCP, the TLS
#' handshake, certificate verification, proxy traversal) without speaking the
#' protocol (\code{CURLOPT_CONNECT_ONLY}), and \code{curl_send()} /
#' \code{curl_recv()} then exchange raw bytes on the established socket
#' (\code{curl_easy_send()} / \code{curl_easy_recv()}). This gives R packages
#' a TLS socket for protocols, or protocol features, that libcurl does not
#' implement itself, for instance IMAP \code{IDLE} (RFC 2177).
#'
#' @param url the url to connect to, e.g. \code{"imaps://imap.example.com"}
#'   or \code{"telnet://example.com:1234"} for a plain TCP connection.
#' @param handle a curl handle (to set timeouts, TLS or proxy options); by
#'   default a new one.
#' @param data a character string or raw vector to send.
#' @param timeout maximum time to wait, in milliseconds.
#' @param max_bytes maximum number of bytes to read in one call.
#' @return \code{curl_connect()} returns the connected handle, invisibly.
#'   \code{curl_send()} returns the number of bytes sent. \code{curl_recv()}
#'   returns a raw vector with the bytes read: empty on timeout, and with the
#'   attribute \code{closed = TRUE} when the peer has closed the connection.
#' @details The connection must be established with \code{curl_connect()}:
#'   a \code{connect_only} handle that went through \code{curl_fetch_memory()}
#'   cannot be used, because the fetch functions perform on the shared multi
#'   handle and remove the easy handle from it afterwards, which detaches the
#'   connection. libcurl consumes the initial exchange of pingpong protocols
#'   (the IMAP greeting and capability list, for example) while connecting,
#'   so it is not returned by the first \code{curl_recv()}. A connect-only
#'   handle cannot be reused for regular transfers.
#' @examples \dontrun{
#' h <- curl_connect("imaps://imap.gmail.com")
#' curl_send(h, "A1 CAPABILITY\r\n")
#' rawToChar(curl_recv(h, timeout = 5000))
#' }
#' @export
#' @useDynLib curl R_curl_connect R_curl_send R_curl_recv
#' @rdname curl_socket
curl_connect <- function(url, handle = new_handle()) {
  stopifnot(is.character(url), length(url) == 1)
  .Call(R_curl_connect, handle, enc2utf8(url))
  invisible(handle)
}

#' @export
#' @rdname curl_socket
curl_send <- function(handle, data, timeout = 30000) {
  if (is.character(data)) data <- charToRaw(paste(data, collapse = ""))
  stopifnot(is.raw(data))
  .Call(R_curl_send, handle, data, as.integer(timeout))
}

#' @export
#' @rdname curl_socket
curl_recv <- function(handle, timeout = 30000, max_bytes = 65536L) {
  .Call(R_curl_recv, handle, as.integer(timeout), as.integer(max_bytes))
}
