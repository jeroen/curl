#' Parse response headers
#'
#' Parse response header data as returned by curl_perform. If the request has
#' followed redirects, the data can contain multiple sets of headers. Therefore
#' when multiple = TRUE, the function returns a list with the response headers
#' for each request. By default it only returns the headers of the final request.
#'
#' @param txt raw or character vector with the header data
#' @param multiple if true, parses multiple sets of headers as separated by
#' a blank line. These are usually the result of redirects. When set to false
#' it only parses the headers from the final request.
#' @export
#' @examples out <- curl_perform("https://httpbin.org/redirect/3")
#' parse_headers(out$headers)
#' parse_headers(out$headers, multiple = TRUE)
parse_headers <- function(txt, multiple = FALSE){
  if(is.raw(txt)){
    txt <- rawToChar(txt)
  }
  stopifnot(is.character(txt))
  if(length(txt) > 1){
    txt <- paste(txt, collapse = "\n")
  }

  # Allow for either "\r\n" line breaks or just "\r" or "\n" (i.e. windows servers)
  sets <- strsplit(txt, "\\r\\n\\r\\n|\\n\\n|\\r\\r")[[1]]
  headers <- strsplit(sets, "\\r\\n|\\n|\\r")
  if(multiple){
    headers
  } else {
    tail(headers, 1)
  }
}
