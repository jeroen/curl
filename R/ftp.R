#' Upload a File
#'
#' Upload a file to e.g. an FTP or SCP server.
#'
#' @export
#' @param file connection object or path to an existing file on disk
#' @param url where to upload, should start with e.g. \code{ftp://}
#' @param verbose emit some progress output
#' @param reuse try to keep alive and recycle connections when possible
#' @param ... other arguments passed to \code{\link{handle_setopt}}, for
#' example a \code{username} and \code{password}.
#' @examples \donttest{# Upload package to winbuilder:
#' curl_upload('mypkg_1.3.tar.gz', 'ftp://win-builder.r-project.org/R-devel/')
#' }
curl_upload <- function(file, url, verbose = TRUE, reuse = TRUE, ...) {
  con <- if(is.character(file)){
    base::file(normalizePath(file, mustWork = TRUE), open = 'rb')
  } else if(inherits(file, 'connection')){
    file
  } else {
    stop("Parameter 'file' must be a ")
  }
  on.exit(close(con))
  total_bytes <- 0
  h <- new_handle(upload = TRUE, filetime = FALSE, readfunction = function(n) {
    buf <- readBin(con, raw(), n = n)
    total_bytes <<- total_bytes + length(buf)
    if(verbose){
      if(length(buf)){
        cat(sprintf("\rUploaded %d bytes...", total_bytes), file = stderr())
      } else {
        cat(sprintf("\rUploaded %d bytes... all done!\n", total_bytes), file = stderr())
      }
    }
    return(buf)
  }, forbid_reuse = !isTRUE(reuse), verbose = verbose)
  if(grepl('/$', url) && is.character(file)){
    url <- paste0(url, basename(file))
  }
  curl_fetch_memory(url, handle = h)
}
