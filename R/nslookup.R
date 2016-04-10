#' Lookup a hostname
#'
#' The \code{nslookup} function is similar to \code{nsl} but works on all platforms
#' and can resolve ipv6 addresses if supported by the OS. Default behavior raises an
#' error if lookup fails. The \code{has_internet} function tests the internet
#' connection by resolving a random address.
#'
#' @export
#' @param host a string with a hostname
#' @param error raise an error for failed DNS lookup. Otherwise returns \code{NULL}.
#' @rdname nslookup
#' @useDynLib curl R_nslookup
#' @examples # Should always work if we are online
#' nslookup("www.r-project.org")
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

#' @export
#' @rdname nslookup
has_internet <- function(){
  !is.null(nslookup("r-project.org", error = FALSE))
}
