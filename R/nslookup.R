#' Lookup a hostname
#'
#' The \code{nslookup} function is similar to \code{nsl} but works on all platforms
#' and can resolve ipv6 addresses if supported by the OS. Default behavior raises an
#' error if lookup fails.
#'
#' The \code{has_internet} function tests for internet connectivity by performing a
#' dns lookup. If a proxy server is detected, it will also check for connectivity by
#' connecting via the proxy.
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
#' @rdname nslookup
has_internet <- local({
  has_internet_via_proxy <- NULL
  function(){
    res <- nslookup("google.com", error = FALSE)
    if(length(res))
      return(TRUE)

    if(length(has_internet_via_proxy))
      return(has_internet_via_proxy)

    # Try via a proxy on Windows
    test_url <- 'https://1.1.1.1'
    ie_proxy <- ie_get_proxy_for_url(test_url)
    proxy_vars <- Sys.getenv(c('ALL_PROXY', 'https_proxy', 'HTTPS_PROXY', 'HTTPS_proxy'), NA)
    handle <- if(any(!is.na(proxy_vars))){
      cat("Testing for internet connectivity via https_proxy... ", file = stderr())
      new_handle(CONNECTTIMEOUT = 5)
    } else if(length(ie_proxy)){
      cat("Testing for internet connectivity via IE proxy... ", file = stderr())
      new_handle(CONNECTTIMEOUT = 5, proxy = ie_proxy)
    } else {
      # Failed to find a proxy server.
      return(FALSE)
    }
    req <- try(curl_fetch_memory(url = test_url, handle = handle), silent = TRUE)
    has_internet_via_proxy <<- is.list(req) && identical(req$status_code, 200L)
    cat(ifelse(has_internet_via_proxy, "success!\n", "failed.\n"), file = stderr())
    return(has_internet_via_proxy)
  }
})
