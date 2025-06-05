#' Normalizing URL parser
#'
#' Interfaces the libcurl [URL parser](https://curl.se/libcurl/c/libcurl-url.html).
#' URLs are automatically normalized where possible, such as in the case of
#' relative paths or url-encoded queries (see examples).
#' When parsing hyperlinks from a HTML document, it is possible to set `baseurl`
#' to the location of the document itself such that relative links can be resolved.
#'
#' A valid URL contains at least a scheme and a host, other pieces are optional.
#' If these are missing, the parser raises an error. Otherwise it returns
#' a list with the following elements:
#'  - *url*: the normalized input URL
#'  - *scheme*: the protocol part before the `://` (required)
#'  - *host*: name of host without port (required)
#'  - *port*: decimal between 0 and 65535
#'  - *path*: normalized path up till the `?` of the url
#'  - *query*: search query: part between the `?` and `#` of the url. Use `params` below to get individual parameters from the query.
#'  - *fragment*: the hash part after the `#` of the url
#'  - *user*: authentication username
#'  - *password*: authentication password
#'  - *params*: named vector with parameters from `query` if set
#'
#' Each element above is either a string or `NULL`, except for `params` which
#' is always a character vector with the length equal to the number of parameters.
#'
#' Note that the `params` field is only usable if the `query` is in the usual
#' `application/x-www-form-urlencoded` format which is technically not part of
#' the RFC. Some services may use e.g. a json blob as the query, in which case
#' the parsed `params` field here can be ignored. There is no way for the parser
#' to automatically infer or validate the query format, this is up to the caller.
#'
#' For more details on the URL format see
#' [rfc3986](https://datatracker.ietf.org/doc/html/rfc3986)
#' or the steps explained in the [whatwg basic url parser](https://url.spec.whatwg.org/#concept-basic-url-parser).
#'
#' On platforms that do not have a recent enough curl version (basically only
#' RHEL-8) the [Ada URL](https://github.com/ada-url/ada) library is used as fallback.
#' Results should be identical, though curl has nicer error messages. This is
#' a temporary solution, we plan to remove the fallback when old systems are
#' no longer supported.
#'
#' @export
#' @param url a character string of length one
#' @param baseurl use this as the parent if `url` may be a relative path
#' @param decode automatically [url-decode][curl_escape] output.
#' Set to `FALSE` to get output in url-encoded format.
#' @param params parse individual parameters assuming query is in `application/x-www-form-urlencoded` format.
#' @useDynLib curl R_parse_url
#' @examples
#' url <- "https://jerry:secret@google.com:888/foo/bar?test=123#bla"
#' curl_parse_url(url)
#'
#' # Resolve relative links from a baseurl
#' curl_parse_url("/somelink", baseurl = url)
#'
#' # Paths get normalized
#' curl_parse_url("https://foobar.com/foo/bar/../baz/../yolo")$url
#'
#' # Also normalizes URL-encoding (these URLs are equivalent):
#' url1 <- "https://ja.wikipedia.org/wiki/\u5bff\u53f8"
#' url2 <- "https://ja.wikipedia.org/wiki/%e5%af%bf%e5%8f%b8"
#' curl_parse_url(url1)$path
#' curl_parse_url(url2)$path
#' curl_parse_url(url1, decode = FALSE)$path
#' curl_parse_url(url1, decode = FALSE)$path
curl_parse_url <- function(url, baseurl = NULL, decode = TRUE, params = TRUE){
  stopifnot(is.character(url))
  stopifnot(length(url) == 1)
  baseurl <- as.character(baseurl)

  # Workaround for #366
  if(length(baseurl) && substr(url, 1, 1) == '#'){
    url <- sub('(#.*)?$', url, baseurl)
  }

  result <- .Call(R_parse_url, url, baseurl)
  if(inherits(result, 'ada')){
    result <- normalize_ada(result)
  }
  # Need to parse query before url-decoding
  if(params){
    tryCatch({
      result$params <- parse_query_urlencoded(result$query)
      result$query <- NULL
    }, error = message)
  }

  if(isTRUE(decode)){
    if(length(result$url))
      result$url <- curl_unescape(result$url)
    if(length(result$path))
      result$path <- curl_unescape(result$path)
    if(length(result$query))
      result$query <- curl_unescape(result$query)
    if(length(result$fragment))
      result$fragment <- curl_unescape(result$fragment)
    if(length(result$user))
      result$user <- curl_unescape(result$user)
    if(length(result$password))
      result$password <- curl_unescape(result$password)
  }
  result
}

#' @details You can use [curl_modify_url()] both to modify an existing URL, or to
#' create new URL from scratch. Arguments get automatically URL-encoded where
#' needed, unless wrapped in `I()`. If `params` is given, this gets converted
#' into a `application/x-www-form-urlencoded` string which overrides `query`.
#' When modifying a URL, use an empty string `""` to unset a piece of the URL.
#' @export
#' @rdname curl_parse_url
#' @useDynLib curl R_modify_url
#' @param url either URL string or list returned by [curl_parse_url].
#' Use this to modify a URL using the other parameters.
#' @param scheme string with e.g. `https`. Required if no `url` parameter was given.
#' @param host string with hostname. Required if no `url` parameter was given.
#' @param port string or number with port, e.g. `"443"`.
#' @param path piece of the url starting with `/` up till `?` or `#`
#' @param query piece of url starting with `?` up till `#`. Only used if no `params` is given.
#' @param fragment part of url starting with `#`.
#' @param user string with username
#' @param password string with password
#' @param params named character vector with http GET parameters. This will automatically
#' be converted to `application/x-www-form-urlencoded` and override `query`,
curl_modify_url <- function(url = NULL, scheme = NULL, host = NULL, port = NULL, path = NULL,
                           query = NULL, fragment = NULL, user = NULL, password = NULL, params = NULL){
  if(is.list(url)){
    url <- do.call(curl_modify_url, url)
  }
  # ADA needs a starting URL. Remove when ADA is removed.
  if(!length(url)){
    url <- sprintf('%s://%s', scheme, host)
  }
  if(length(params) > 0){
    query <- I(build_query_urlencoded(params))
  }
  port <- as.character(port)
  .Call(R_modify_url, url, scheme, host, port, path, query, fragment, user, password);
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

# Parses a string in 'application/x-www-form-urlencoded' format
parse_query_urlencoded <- function(query){
  if(!length(query)) return(character())
  query <- chartr('+',' ', query)
  argstr <- strsplit(query, "&", fixed = TRUE)[[1]]
  args <- lapply(argstr, function(x){
    c(curl_unescape(strsplit(x, "=", fixed = TRUE)[[1]]), "")
  })
  values <- vapply(args, `[`, character(1), 2)
  names(values) <- vapply(args, `[`, character(1), 1)
  return(values)
}

build_query_urlencoded <- function(params){
  if(!is.character(params) || length(names(params)) != length(params)){
    stop("params must be named character vector")
  }
  nms <- curl_escape(names(params))
  values <- gsub("%20", "+", curl_escape(params), fixed = TRUE)
  paste(nms, values, collapse = '&', sep = '=')
}

try_parse_url <- function(url){
  tryCatch(curl_parse_url(url), error = function(e){})
}
