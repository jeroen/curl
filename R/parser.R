#' Parse a URL
#'
#' Wrapper for the libcurl [URL parsing interface](https://curl.se/libcurl/c/libcurl-url.html).
#' This requires at least libcurl 7.62.
#'
#' @export
#' @param url a character string of length one
#' @useDynLib curl R_parse_url
parse_url <- function(url){
  stopifnot(is.character(url))
  .Call(R_parse_url, url)
}
