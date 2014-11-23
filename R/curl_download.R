#' Download file
#'
#' Libcurl implementation of \code{C_download} (the "internal" download method).
#' Designed to behave similar to \code{\link{download.file}}.
#'
#' @useDynLib curl R_download_curl R_download_cleanup
#' @param url A character string naming the URL of a resource to be downloaded.
#' @param destfile A character string with the name where the downloaded file is saved.
#' Tilde-expansion is performed.
#' @param quiet If \code{TRUE}, suppress status messages (if any), and the progress bar.
#' @param mode A character string specifying the mode with which to write the file. Useful values are \code{"w"},
#' \code{"wb"} (binary), \code{"a"} (append) and \code{"ab"}.
#' @export
#' @examples \dontrun{download large file
#' url <- "http://www2.census.gov/acs2011_5yr/pums/csv_pus.zip"
#' tmp <- tempfile()
#' curl_download(url, tmp)
#' }
curl_download <- function(url, destfile, quiet = FALSE, mode = "w"){
  destfile <- normalizePath(destfile, mustWork = FALSE)
  if(file.exists(destfile)){
    newfile <- FALSE
  } else {
    stopifnot(file.create(destfile))
    newfile <- TRUE
  }
  destfile <- normalizePath(destfile, mustWork = TRUE)
  tryCatch({
    # Clean in case of interrupt
    on.exit(.Call(R_download_cleanup))
    invisible(.Call(R_download_curl, url, destfile, quiet, mode))
  }, error = function(err){
    # Need to close descriptor before we can unlink
    if(isTRUE(newfile)){
      .Call(R_download_cleanup)
      unlink(destfile)
    }
    stop(err$message, call. = FALSE)
  });
}

#' @useDynLib curl R_global_cleanup
.onUnload <- function(lib){
  .Call(R_global_cleanup);
}
