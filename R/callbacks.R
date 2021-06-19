#' @useDynLib curl R_set_cb_handlers
init_callback_error_handlers <- function(){
  .Call(R_set_cb_handlers, callback_wrapper, raise_last_error)
}

callback_wrapper <- function(callback, ...){
  save_error(NULL)
  withCallingHandlers(callback(...), error = function(e){
    save_error(e)
  })
}

errenv <- new.env(parent = emptyenv())

save_error <- function(e){
  errenv$e <- e
}

raise_last_error <- function(){
  if(!is.null(errenv$e)){
    e <- errenv$e
    errenv$e <- NULL
    e$call = NULL
    stop(e)
  }
}
