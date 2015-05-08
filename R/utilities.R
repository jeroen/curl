#' Determine curl version.
#'
#' @useDynLib curl R_curl_version
#' @export
#' @examples
#' # Curl/ssl version info
#' curl_version()
curl_version <- function(){
  .Call(R_curl_version);
}

#' List all curl options.
#'
#' \code{curl_options()} lists all options available in the current version
#' of libcurl.  The dataset \code{curl_symbols} lists all symbols (including
#' options) provides more information about the symbols, including when support
#' was added/removed from libcurl.
#'
#' @useDynLib curl R_curl_options
#' @export
#' @examples
#' # List available curl options
#' curl_options()
#' # See all symbols
#' head(curl_symbols)
curl_options <- function(){
  .Call(R_curl_options);
}

#' @rdname curl_options
#' @format A data frame with columns:
#' \describe{
#' \item{name}{Symbol name}
#' \item{introduced,deprecated,removed}{Versions of libcurl}
#' \item{value}{Integer value of symbol}
#' \item{type}{If an option, the type of value it needs}
#' }
"curl_symbols"

#' Parse date/times.
#'
#' @param datestring a string consisting of a timestamp
#' @useDynLib curl R_curl_getdate
#' @export
#' @examples
#' # Parse dates in many formats
#' curl_getdate("Sunday, 06-Nov-94 08:49:37 GMT")
#' curl_getdate("06 Nov 1994 08:49:37")
#' curl_getdate("20040911 +0200")
curl_getdate <- function(datestring){
  out <- .Call(R_curl_getdate, datestring);
  class(out) <- c("POSIXct", "POSIXt")
  out
}
