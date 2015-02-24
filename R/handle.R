#' @useDynLib curl R_new_handle
#' @export
new_handle <- function(){
  .Call(R_new_handle)
}
