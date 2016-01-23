#' Lookup a hostname
#'
#' Similar to \code{nsl} but works on all platforms and can resolve ipv6
#' addresses on supported platforms. Default behavior raises an error if
#' lookup fails.
#'
#' @export
#' @param host a string with a hostname
#' @param error raise an error for failed DNS lookup. Otherwise returns \code{NULL}.
#' @useDynLib curl R_nslookup
#' @examples nslookup("www.r-project.org")
#'
#' # If your OS supports IPv6
#' nslookup("ipv6.test-ipv6.com", error = FALSE)
nslookup <- function(host, error = TRUE){
  stopifnot(is.character(host))
  if(grepl("://", host, fixed = TRUE))
    stop("This looks like a URL, not a hostname")
  out <- .Call(R_nslookup, host[1])
  if(isTRUE(error) && is.null(out))
    stop("Unable to resolve host: ", host)
  out
}
