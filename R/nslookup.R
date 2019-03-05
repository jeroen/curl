#' Lookup a hostname
#'
#' The \code{nslookup} function is similar to \code{nsl} but works on all platforms
#' and can resolve ipv6 addresses if supported by the OS. Default behavior raises an
#' error if lookup fails. The \code{has_internet} function tests the internet
#' connection by resolving a random address if \code{handle} parameter is null, or
#' querying a random address with the provided \code{handle}.
#'
#' @export
#' @param host a string with a hostname
#' @param error raise an error for failed DNS lookup. Otherwise returns \code{NULL}.
#' @param ipv4_only always return ipv4 address. Set to `FALSE` to allow for ipv6 as well.
#' @param multiple returns multiple ip addresses if possible
#' @rdname nslookup
#' @useDynLib curl R_nslookup
#' @examples # Should always work if we are online
#' nslookup("www.r-project.org")
#'
#' # If your OS supports IPv6
#' nslookup("ipv6.test-ipv6.com", ipv4_only = FALSE, error = FALSE)
nslookup <- function(host, ipv4_only = FALSE, multiple = FALSE, error = TRUE){
  stopifnot(is.character(host))
  host <- enc2utf8(host)
  if(grepl("://", host, fixed = TRUE))
    stop("This looks like a URL, not a hostname")
  out <- .Call(R_nslookup, host[1], as.logical(ipv4_only))
  if(isTRUE(error) && is.null(out))
    stop("Unable to resolve host: ", host)
  if(isTRUE(multiple))
    return(unique(out))
  utils::head(out, 1)
}

#' @export
#' @param handle a curl handle object (for \code{has_internet})
#' @rdname nslookup
has_internet <- function(handle = NULL){
  if(is.null(handle){
    !is.null(nslookup("google.com", error = FALSE))
  } else {
    !inherits(try(curl_fetch_memory('https://www.google.com', handle = handle)), "try-error")
  }
}
