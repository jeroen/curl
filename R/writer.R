#' File Writer
#'
#' Generates a closure that writes binary (raw) data to a file.
#'
#' The writer function automatically opens the file on the first write and closes when
#' it goes out of scope, or explicitly by setting \code{close = TRUE}. This can be used
#' for the \code{data} callback in \code{multi_add()} or \code{curl_fetch_multi()}.
#'
#' @export
#' @param path file name or path on disk
#' @return Function with signature \code{writer(data = raw(), close = FALSE)}
#' @examples
#' # Doesn't open yet
#' tmp <- tempfile()
#' writer <- file_writer(tmp)
#'
#' # Now it opens
#' writer(charToRaw("Hello!\n"))
#' writer(charToRaw("How are you?\n"))
#'
#' # Close it!
#' writer(charToRaw("All done!\n"), close = TRUE)
#'
#' # Check it worked
#' readLines(tmp)
file_writer <- function(path){
  path <- normalizePath(path, mustWork = FALSE)
  fp <- new_file_writer(path)
  structure(function(data = raw(), close = FALSE){
    stopifnot(is.raw(data))
    write_file_writer(fp, data, as.logical(close))
  }, class = "file_writer")
}

#' @useDynLib curl R_new_file_writer
new_file_writer <- function(path){
  .Call(R_new_file_writer, path)
}

#' @useDynLib curl R_write_file_writer
write_file_writer <- function(fp, data, close){
  .Call(R_write_file_writer, fp, data, close)
}

#' @useDynLib curl R_total_writers
total_writers <- function(){
  .Call(R_total_writers)
}
