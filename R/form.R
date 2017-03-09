#' POST files or data
#'
#' Build multipart form data elements. The \code{form_file} function uploads a
#' file. The \code{form_data} function allows for posting a string or raw vector
#' with a custom content-type.
#'
#' @param path a string with a path to an existing file on disk
#' @param type MIME content-type of the file.
#' @export
#' @name multipart
#' @rdname multipart
form_file <- function(path, type = NULL){
  path <- normalizePath(path[1], mustWork = TRUE)
  if(!is.null(type)){
    stopifnot(is.character(type))
  }
  structure(list(path = path, type = type), class = "formdata")
}

#' @export
#' @name multipart
#' @rdname multipart
#' @param value a character or raw vector to post
form_data <- function(value, type = NULL){
  if(is.character(value))
    value <- charToRaw(paste(enc2utf8(value), collapse = "\n"))
  if(!is.raw(value))
    stop("Argument 'value' must be string or raw vector")
  structure(list(value = value, type = type), class = "formdata")
}

#' @export
print.formdata <- function(x, ...){
  txt <- if(is.character(x[[1]])){
    paste("Form file:", basename(x$path))
  } else {
    paste("Form data: <<data>>")
  }
  if(!is.null(x$type)){
    txt <- paste0(txt, " (", x$type, ")")
  }
  cat(txt, "\n")
}
