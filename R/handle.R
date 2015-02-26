#' Create new libcurl handle
#'
#' This function creates a new libcurl easy handle object.
#'
#' @useDynLib curl R_new_handle
#' @export
new_handle <- function(){
  .Call(R_new_handle)
}
