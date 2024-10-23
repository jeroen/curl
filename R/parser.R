#' Parse a URL
#'
#' Simple wrapper for the libcurl [URL parsing interface](https://curl.se/libcurl/c/libcurl-url.html).
#'
#' When parsing hyperlinks inside a HTML document, it is possible to set `baseurl`
#' to the location of the document such that relative links can be resolved.
#'
#' @export
#' @param url a character string of length one
#' @param baseurl if url is a relative path, this url is used as the parent.
#' @useDynLib curl R_parse_url
#' @examples
#' url <- "https://jerry:secret@google.com:888/foo/bar?test=123#bla"
#' parse_url(url)
#'
#' # Resolve relative links
#' parse_url("/somelink", baseurl = url)
parse_url <- function(url, baseurl = NULL){
  stopifnot(is.character(url))
  baseurl < as.character(baseurl)
  result <- .Call(R_parse_url, url, baseurl)
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
