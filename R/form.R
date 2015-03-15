#' Upload files with form
#'
#' @param path a string with a path to an existing file on disk
#' @param type MIME content-type of the file.
#' @export
form_file <- function(path, type = NULL){
  path <- normalizePath(path[1], mustWork = TRUE)
  if(!is.null(type)){
    stopifnot(is.character(type))
  }
  out <- list(path = path, type = type)
  class(out) <- "form_file"
  out
}

#' @export
print.form_file <- function(x, ...){
  txt <- paste("Form file:", basename(x$path))
  if(!is.null(x$type)){
    txt <- paste0(txt, " (", x$type, ")")
  }
  cat(txt, "\n")
}
