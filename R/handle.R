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
#' req <- curl_perform("http://httpbin.org/cookies/set?foo=123&bar=ftw", handle = h)
#'
#' # See http://www.cookiecentral.com/faq/#3.5
#' get_handle_cookies(h)
get_handle_cookies <- function(handle){
  cookies <- .Call(R_get_handle_cookies, handle)
  df <- if(length(cookies)){
    as.data.frame(do.call(rbind, strsplit(cookies, split="\t")))
  } else {
    as.data.frame(matrix(ncol=7, nrow=0))
  }
  names(df) <- c("domain", "flag", "path", "secure", "expiration", "name", "value")
  df$flag <- as.logical(df$flag)
  df$secure <- as.logical(df$secure)
  df
}
