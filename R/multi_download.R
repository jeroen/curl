#' Advanced download interface
#'
#' Download multiple files concurrently, with support for resuming large files.
#' This function is based on [multi_run()] and hence does not error in case any
#' of the individual requests fail; you should inspect the return value to find
#' out which of the downloads were completed successfully.
#'
#' Upon completion of all requests, this function returns a data frame with results.
#' The `success` column indicates if a request was successfully completed (regardless
#' of the HTTP status code). If it failed, e.g. due to a networking issue, the error
#' message is in the `error` column. A `success` value `NA` indicates that the request
#' was still in progress when the function was interrupted or reached the elapsed
#' `timeout` and perhaps the download can be resumed if the server supports it.
#'
#' It is also important to inspect the `status_code` column to see if any of the
#' requests were successful but had a non-success HTTP code, and hence the downloaded
#' file probably contains an error page instead of the requested content.
#'
#' Note that when you set `resume = TRUE` you should expect HTTP-206 or HTTP-416
#' responses. The latter could indicate that the file was already complete, hence
#' there was no content left to resume from the server. If you try to resume a file
#' download but the server does not support this, success if `FALSE` and the file
#' will not be touched. In fact, if we request to a download to be resumed and the
#' server responds `HTTP 200` instead of `HTTP 206`, libcurl will error and not
#' download anything, because this probably means the server did not respect our
#' range request and is sending us the full file.
#'
#' @returns The function returns a data frame with one row for each downloaded file and
#' the following columns:
#'  - `success` if the HTTP request was successfully performed, regardless of the
#'  response status code. This is `FALSE` in case of a network error, or in case
#'  you tried to resume from a server that did not support this. A value of `NA`
#'  means the download was interrupted while in progress.
#'  - `status_code` the HTTP status code from the request. A successful download is
#'  usually `200` for full requests or `206` for resumed requests. Anything else
#'  could indicate that the downloaded file contains an error page instead of the
#'  requested content.
#'  - `resumefrom` the file size before the request, in case a download was resumed.
#'  - `url` final url (after redirects) of the request.
#'  - `destfile` downloaded file on disk.
#'  - `error` if `success == FALSE` this column contains an error message.
#'  - `type` the `Content-Type` response header value.
#'  - `modified` the `Last-Modified` response header value.
#'  - `time` total elapsed download time for this file in seconds.
#'  - `headers` vector with http response headers for the request.
#'
#' @export
#' @param urls vector with files to download
#' @param destfiles vector (of equal length as `urls`) with paths of output files,
#' or `NULL` to use [basename] of urls.
#' @param resume if the file already exists, resume the download. Note that this may
#' change server responses, see details.
#' @param timeout in seconds, passed to [multi_run]
#' @param progress print download progress information
#' @param ... extra handle options passed to each request [new_handle]
#' @examples \dontrun{
#' urls <- c('https://cran.r-project.org/src/contrib/Archive/V8/V8_4.2.1.tar.gz',
#' 'https://cran.r-project.org/src/contrib/Archive/curl/curl_4.3.2.tar.gz',
#' 'https://urldoesnotexist.xyz/nothing.zip',
#' 'https://github.com/jeroen/curl/archive/refs/heads/master.zip',
#' 'https://httpbin.org/status/418')
#'
#' multi_download(urls)
#' }
multi_download <- function(urls, destfiles = NULL, resume = FALSE, progress = TRUE, timeout = Inf, ...){
  urls <- enc2utf8(urls)
  if(is.null(destfiles)){
    destfiles <- basename(sub("[?#].*", "", urls))
  }
  stopifnot(length(urls) == length(destfiles))
  destfiles <- normalizePath(destfiles, mustWork = FALSE)
  handles <- rep(list(NULL), length(urls))
  writers <- rep(list(NULL), length(urls))
  errors <- rep(NA_character_, length(urls))
  success <- rep(NA, length(urls))
  resumefrom <- rep(0, length(urls))
  dlspeed <- rep(0, length(urls))
  expected <- rep(NA, length(urls))
  pool <- new_pool()
  total <- 0
  lapply(seq_along(urls), function(i){
    dest <- destfiles[i]
    handle <- new_handle(url = urls[i], ...)
    handle_setopt(handle, noprogress = TRUE)
    if(isTRUE(resume) && file.exists(dest)){
      startsize <- file.info(dest)$size
      handle_setopt(handle, resume_from_large = startsize)
      total <<- total + startsize
      resumefrom[i] <- startsize
    }
    writer <- file_writer(dest, append = resume)
    multi_add(handle, pool = pool, data = function(buf, final){
      total <<- total + length(buf)
      writer(buf, final)
      if(isTRUE(progress)){
        if(is.na(expected[i])){
          expected[i] <<- handle_clength(handle) + resumefrom[i]
        }
        dlspeed[i] <<- ifelse(final, 0, handle_speed(handle)[1])
        print_progress(success, total, sum(dlspeed), sum(expected))
      }
    }, done = function(req){
      expected[i] <<- handle_received(handle) + resumefrom[i]
      success[i] <<- TRUE
      dlspeed[i] <<- 0
    }, fail = function(err){
      expected[i] <<- handle_received(handle) + resumefrom[i]
      success[i] <<- FALSE
      errors[i] <<- err
      dlspeed[i] <<- 0
    })
    handles[[i]] <<- handle
    writers[[i]] <<- writer
    if(isTRUE(progress) && (i %% 100 == 0)){
      print_stream("\rPreparing request %d of %d...", i, length(urls))
    }
  })
  on.exit(lapply(writers, function(writer){
    # fallback to close writer in case the download got interrupted
    writer(raw(0), close = TRUE)
  }))
  tryCatch({
    multi_run(timeout = timeout, pool = pool)
    if(isTRUE(progress)){
      print_progress(success, total, sum(dlspeed), sum(expected), TRUE)
    }
  }, interrupt = function(e){
    message("download interrupted")
  })
  out <- lapply(handles, handle_data)
  results <- data.frame(
    success = success,
    status_code = sapply(out, function(x){x$status_code}),
    resumefrom = resumefrom,
    url = sapply(out, function(x){x$url}),
    destfile = destfiles,
    error = errors,
    type = sapply(out, function(x){x$type}),
    modified = structure(sapply(out, function(x){x$modified}), class = c("POSIXct", "POSIXt")),
    time = sapply(out, function(x){unname(x$times['total'])}),
    stringsAsFactors = FALSE
  )
  results$headers <- lapply(out, function(x){parse_headers(x$headers)})
  class(results) <- c("tbl_df", "tbl", "data.frame")
  results
}

# Print at most 10x per second in interactive, and once per sec in batch/CI
print_progress <- local({
  last <- 0
  function(sucvec, total, speed, expected, finalize = FALSE){
    throttle <- ifelse(interactive(), 0.1, 5)
    now <- unclass(Sys.time())
    if(isTRUE(finalize) || now - last > throttle){
      last <<- now
      done <- sum(!is.na(sucvec))
      pending <- sum(is.na(sucvec))
      pctstr <- sprintf("(%s%%)", ifelse(is.na(expected), "??", as.character(round(100 * total/expected))))
      speedstr <- if(!finalize){
        sprintf(" (%s/s)", format_size(speed))
      } else {""}
      downloaded <- format_size(total)
      print_stream('\rDownload status: %d done; %d in progress%s. Total size: %s %s...',
                   done, pending, speedstr, downloaded, pctstr)
    }
    if(finalize){
      cat(" done!             \n", file = stderr())
      flush(stderr())
    }
  }
})

print_stream <- function(...){
  cat(sprintf(...), file = stderr())
}
