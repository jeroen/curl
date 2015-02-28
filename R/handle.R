#' Create new libcurl handle
#'
#' Functions to create and manipualte handle objects. Note that currently
#' handle_setopt will append options whereas handle_setheader will reset
#' all of the currently set headers.
#'
#' @useDynLib curl R_new_handle
#' @export
#' @rdname handle
new_handle <- function(){
  h <- .Call(R_new_handle)
  handle_setopt(h,
    SSL_VERIFYHOST = FALSE,
    SSL_VERIFYPEER = FALSE,
    FOLLOWLOCATION = TRUE,
    CONNECTTIMEOUT_MS = 10*1000
  )

  handle_setheader(h,
    "User-Agent" = "r/curl/jeroen",
    "Accept-Charset" = "utf-8"
  )
  return(h)
}

#' @export
#' @rdname handle
#' @param ... additional options / headers to be set in the handle.
#' @useDynLib curl R_handle_setopt
handle_setopt <- function(handle, ...){
  values <- list(...)
  keys <- as.integer(curl_options()[names(values)])
  if(anyNA(keys)){
    stop("Unknown options.")
  }
  stopifnot(length(keys) == length(values))
  .Call(R_handle_setopt, handle, keys, values)
}

#' @export
#' @useDynLib curl R_handle_setheader
#' @rdname handle
handle_setheader <- function(handle, ...){
  opts <- list(...)
  names <- names(opts)
  values <- as.character(unlist(opts))
  vec <- paste0(names, ": ", values)
  .Call(R_handle_setheader, handle, vec)
}


#' @useDynLib curl R_get_handle_cookies
#' @export
#' @rdname handle
#' @param handle a curl handle object
#' @examples h <- new_handle()
#' get_handle_cookies(h)
#'
#' # Server sets cookies
#' req <- curl_perform("http://httpbin.org/cookies/set?foo=123&bar=ftw", handle = h)
#' get_handle_cookies(h)
#'
#' # Server deletes cookies
#' req <- curl_perform("http://httpbin.org/cookies/delete?foo", handle = h)
#' get_handle_cookies(h)
get_handle_cookies <- function(handle){
  cookies <- .Call(R_get_handle_cookies, handle)
  df <- if(length(cookies)){
    values <- lapply(strsplit(cookies, split="\t"), `[`, 1:7)
    as.data.frame(do.call(rbind, values), stringsAsFactors = FALSE)
  } else {
    as.data.frame(matrix(ncol=7, nrow=0))
  }
  names(df) <- c("domain", "flag", "path", "secure", "expiration", "name", "value")
  df$flag <- as.logical(df$flag)
  df$secure <- as.logical(df$secure)
  df$expiration <- as.numeric(df$expiration)
  df
}
