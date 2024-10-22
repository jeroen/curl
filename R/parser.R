#' Parse a URL
#'
#' Wrapper for the libcurl [URL parsing interface](https://curl.se/libcurl/c/libcurl-url.html).
#'
#'
#' @export
#' @param url a character string of length one
#' @useDynLib curl R_parse_url
#' @examples
#' url <- "https://jerry:secret@google.com:888/foo/bar?test=123#bla"
#' parse_url(url)
parse_url <- function(url){
  stopifnot(is.character(url))
  result <- .Call(R_parse_url, url)
  if(inherits(result, 'ada')){
    if(length(result$scheme))
      result$scheme <- sub("\\:$", "", result$scheme)
    if(length( result$query))
      result$query <- sub("^\\?", "", result$query)
    if(length(result$fragment))
      result$fragment <- sub("^\\#", "", result$fragment)
    result <- unclass(result)
  }
  result
}
