#' Lookup a hostname
#'
#' The same thing as \link{nsl} but supports IPv6 and works on all platforms.
#'
#' @export
#' @param hostname a string with a hostname
#' @useDynLib curl R_nslookup
#' @examples nslookup("www.r-project.org")
#' nslookup("ipv6.test-ipv6.com")
nslookup <- function(host){
  stopifnot(is.character(host))
  if(grepl("://", host, fixed = TRUE))
    stop("This looks like a URL, not a hostname")
  .Call(R_nslookup, host[1])
}
