#' @useDynLib curl R_proxy_info
#' @export
proxy_info <- function(){
  .Call(R_proxy_info)
}
