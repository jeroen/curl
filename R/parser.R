#' Normalizing URL parser
#'
#' Interfaces the libcurl [URL parser](https://curl.se/libcurl/c/libcurl-url.html).
#' URLs are automatically normalized, for example for input that is url-encoded or
#' contains paths with `../`, see examples.
#' When parsing hyperlinks inside a HTML document, it is possible to set `baseurl`
#' to the location of the document such that relative links can be resolved.
#'
#' A valid URL requires at least a scheme and a host, other parts are optional.
#' If the input URL is invalid, the function raises an error. Otherwise it returns
#' a list with the following elements:
#'  - *url*: the normalized input URL
#'  - *scheme*: the protocol part before the `://` (required)
#'  - *host*: name of host only (required)
#'  - *port*: decimal between 0 and 65535
#'  - *path*: normalized (url-decoded) path up till the `?` of the url
#'  - *query*: aka search, part between the `?` and `#` of the url
#'  - *fragment*: the hash part after the `#` of the url
#'  - *user*: authentication username
#'  - *password*: authentication password
#'
#' Elements that are not present in the URL are set to `NULL`.
#'
#' For more details on the URL format see
#' [rfc3986](https://datatracker.ietf.org/doc/html/rfc3986)
#' or the steps explained in the [whatwg basic url parser](https://url.spec.whatwg.org/#concept-basic-url-parser).
#'
#' On platforms that do not have a recent enough curl version (basically only
#' RHEL-8) the [Ada URL](https://www.ada-url.com/) library is used as fallback.
#' Results should be identical, though curl has much nicer error messages.
#'
#' @export
#' @param url a character string of length one
#' @param baseurl if url is a relative path, this url is used as the parent.
#' @param decode return [url-decoded][curl_escape] results.
#' Set to `FALSE` to get results in url-encoded format.
#' @useDynLib curl R_parse_url
#' @examples
#' url <- "https://jerry:secret@google.com:888/foo/bar?test=123#bla"
#' parse_url(url)
#'
#' # Resolve relative links from a baseurl
#' parse_url("/somelink", baseurl = url)
#'
#' # Paths get normalized
#' parse_url("https://foobar.com/foo/bar/../baz/../yolo")$url
#'
#' # Also normalizes URL-encoding (these URLs are equivalent):
#' url1 <- "https://ja.wikipedia.org/wiki/\u5bff\u53f8"
#' url2 <- "https://ja.wikipedia.org/wiki/%e5%af%bf%e5%8f%b8"
#' parse_url(url1)$path
#' parse_url(url2)$path
#' parse_url(url1, decode = FALSE)$path
#' parse_url(url1, decode = FALSE)$path
parse_url <- function(url, baseurl = NULL, decode = TRUE){
  stopifnot(is.character(url))
  baseurl < as.character(baseurl)
  result <- .Call(R_parse_url, url, baseurl)
  if(inherits(result, 'ada')){
    result <- normalize_ada(result)
  }
  if(isTRUE(decode)){
    result <- normalize_all(result)
  }
  result
}

normalize_all <- function(result){
  lapply(result, function(x){
    if(length(x)) curl_unescape(x)
  })
}


# NB: Ada also automatically removes the 'port' if it is the default
# for that scheme such as https://host:443. I don't think we can prevent that.
normalize_ada <- function(result){
  if(length(result$scheme))
    result$scheme <- sub("\\:$", "", result$scheme)
  if(length(result$query))
    result$query <- sub("^\\?", "", result$query)
  if(length(result$fragment))
    result$fragment <- sub("^\\#", "", result$fragment)
  unclass(result)
}
