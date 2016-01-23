#' Lookup a hostname
#'
#' The same thing as \link{nsl} but supports IPv6 and works on all platforms.
#'
#' @export
#' @param hostname a string with a host
#' @useDynLib curl R_nslookup
nslookup <- function(host){
  stopifnot(is.character(host))
  .Call(R_nslookup, host[1])
}
