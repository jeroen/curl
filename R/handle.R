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
  .Call(R_new_handle)
}

#' @export
#' @rdname handle
#' @param ... additional options / headers to be set in the handle.
#' @useDynLib curl R_handle_setopt
handle_setopt <- function(handle, ...){
  values <- list(...)
  keys <- as.integer(curl_options()[toupper(names(values))])
  if(anyNA(keys)){
    stop("Unknown options.")
  }
  stopifnot(length(keys) == length(values))
  .Call(R_handle_setopt, handle, keys, values)
  invisible(handle)
}

#' @export
#' @rdname handle
#' @useDynLib curl R_handle_reset
handle_reset <- function(handle){
  .Call(R_handle_reset, handle)
  invisible(handle)
}

#' @export
#' @useDynLib curl R_handle_setheaders
#' @rdname handle
handle_setheaders <- function(handle, ...){
  opts <- list(...)
  if(!all(vapply(opts, is.character, logical(1)))){
    stop("All headers must me strings.")
  }
  names <- names(opts)
  values <- as.character(unlist(opts))
  vec <- paste0(names, ": ", values)
  .Call(R_handle_setheaders, handle, vec)
  invisible(handle)
}

#' @export
#' @useDynLib curl R_handle_setform
#' @rdname handle
handle_setform <- function(handle, ...){
  form <- list(...)
  for(i in seq_along(form)){
    val <- form[[i]];
    if(!is.character(val) && !is.raw(val) && !is(val, "form_file")){
      stop("Insupported value type for form field '", names(form[i]), "'.")
    }
  }
  .Call(R_handle_setform, handle, form)
  invisible(handle)
}

#' @useDynLib curl R_get_handle_cookies
#' @export
#' @rdname handle
#' @param handle a curl handle object
#' @examples h <- new_handle()
#' handle_cookies(h)
#'
#' # Server sets cookies
#' req <- curl_perform("http://httpbin.org/cookies/set?foo=123&bar=ftw", handle = h)
#' handle_cookies(h)
#'
#' # Server deletes cookies
#' req <- curl_perform("http://httpbin.org/cookies/delete?foo", handle = h)
#' handle_cookies(h)
handle_cookies <- function(handle){
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
  expires <- as.numeric(df$expiration)
  expires[expires==0] <- Inf
  class(expires) = c("POSIXct", "POSIXt");
  df$expiration <- expires
  df
}
