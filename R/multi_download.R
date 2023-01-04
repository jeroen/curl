#' Download multiple files concurrently
#'
#' Wrapper for [multi_run()] to download multiple requests concurrently. Also supports
#' resuming downloads for large files. This function does not error in case any of the
#' requests fail; instead it returns a data frame with information about the requests.
#'
#' @export
#' @param urls vector with files to download
#' @param destfiles vector (of equal length as `urls`) with paths of output files,
#' or `NULL` to use [basename] of urls.
#' @param resume if the file already exists, resume the download. Note that this
#' servers can return http errors for resources that cannot be resumed or if the download
#' was already completed, i.e. nothing left to resume.
#' @param timeout in seconds, passed to [multi_run]
#' @param ... extra handle options passed to [new_handle]
#' @examples urls <- c('https://cran.r-project.org/src/contrib/Archive/V8/V8_4.2.1.tar.gz',
#' 'https://cran.r-project.org/src/contrib/Archive/curl/curl_4.3.2.tar.gz',
#' 'https://urldoesnotexist.xyz/nothing.zip',
#' 'https://github.com/jeroen/curl/archive/refs/heads/master.zip',
#' 'https://httpbin.org/status/418')
#'
#' multi_download(urls)
multi_download <- function(urls, destfiles = NULL, resume = FALSE, timeout = Inf, ...){
  if(is.null(destfiles)){
    destfiles <- basename(sub("[?#].*", "", urls))
  }
  stopifnot(length(urls) == length(destfiles))
  destfiles <- normalizePath(destfiles, mustWork = FALSE)
  handles <- rep(list(NULL), length(urls))
  writers <- rep(list(NULL), length(urls))
  errors <- rep(NA_character_, length(urls))
  pool <- new_pool()
  lapply(seq_along(urls), function(i){
    dest <- destfiles[i]
    handle <- new_handle(url = urls[i], ...)
    if(isTRUE(resume) && file.exists(dest)){
      handle_setopt(handle, resume_from_large = file.info(dest)$size)
    }
    writer <- file_writer(dest, append = resume)
    multi_add(handle, pool = pool, data = function(buf, final){
      writer(buf, final)
    }, fail = function(err){
      errors[i] <<- err
    })
    handles[[i]] <<- handle
    writers[[i]] <<- writer
  })
  on.exit(lapply(writers, function(writer){
    # fallback to close writer in case the download got interrupted
    writer(raw(0), close = TRUE)
  }))
  status <- multi_run(timeout = timeout, pool = pool)
  out <- lapply(handles, handle_data)
  df <- data.frame(
    success = is.na(errors),
    status_code = sapply(out, function(x){x$status_code}),
    url = sapply(out, function(x){x$url}),
    destfile = destfiles,
    error = errors,
    type = sapply(out, function(x){x$type}),
    modified = structure(sapply(out, function(x){x$modified}), class = c("POSIXct", "POSIXt")),
    time = sapply(out, function(x){unname(x$times['total'])}),
    stringsAsFactors = FALSE
  )
  df$headers <- lapply(out, function(x){parse_headers(x$headers)})
  class(df) <- c("tbl_df", "tbl", "data.frame")
  list(status = status, results = df)
}
