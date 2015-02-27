#' @useDynLib curl R_proxy_info
#' @export
ie_proxy_info <- function(){
  .Call(R_proxy_info)
}
