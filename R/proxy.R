#' Internet Explorer proxy settings
#'
#' This function retrieves the system proxy settings on Windows as
#' configured in Internet Explorer. This can be used to automatically
#' configure the libcurl to use the same proxy server.
#'
#' @useDynLib curl R_proxy_info
#' @export
ie_proxy_info <- function(){
  .Call(R_proxy_info)
}
