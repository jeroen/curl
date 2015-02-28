#' Create new libcurl handle
#'
#' This function creates a new libcurl easy handle object.
#'
#' @useDynLib curl R_new_handle
#' @export
#' @rdname handle
new_handle <- function(){
  .Call(R_new_handle)
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
