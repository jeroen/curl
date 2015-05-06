#' Download file
#'
#' Libcurl implementation of \code{C_download} (the "internal" download method).
#' Designed to behave similar to \code{\link{download.file}}.
#'
#' @useDynLib curl R_download_curl
#' @param url A character string naming the URL of a resource to be downloaded.
#' @param destfile A character string with the name where the downloaded file
#'   is saved. Tilde-expansion is performed.
#' @param quiet If \code{TRUE}, suppress status messages (if any), and the
#'   progress bar.
#' @param mode A character string specifying the mode with which to write the file.
#'   Useful values are \code{"w"}, \code{"wb"} (binary), \code{"a"} (append)
#'   and \code{"ab"}.
#' @param handle a curl handle object
#' @return Path of downloaded file (invisibly).
#' @export
#' @examples \dontrun{download large file
#' url <- "http://www2.census.gov/acs2011_5yr/pums/csv_pus.zip"
#' tmp <- tempfile()
#' curl_download(url, tmp)
#' }
curl_download <- function(url, destfile, quiet = TRUE, mode = "wb", handle = new_handle()){
  destfile <- normalizePath(destfile, mustWork = FALSE)
  .Call(R_download_curl, url, destfile, quiet, mode, handle)
  invisible(destfile)
}

#' @useDynLib curl R_global_cleanup
.onUnload <- function(lib){
  .Call(R_global_cleanup);
}
