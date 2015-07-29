#' Internet Explorer proxy settings
#'
#' This function retrieves the system proxy settings on Windows as
#' set in Internet Explorer. This can be used to configure curl to
#' use the same proxy server.
#'
#' @useDynLib curl R_proxy_info
#' @export
ie_proxy_info <- function(){
  .Call(R_proxy_info)
}

#' @useDynLib curl R_get_proxy_for_url
ie_auto_proxy <- function(target_url, autoproxy_url){
  .Call(R_get_proxy_for_url, target_url, autoproxy_url)
}
