#' Perform a request
#'
#' @useDynLib curl R_curl_fetch_memory
#' @param url A character string naming the URL of a resource to be downloaded.
#' @param handle a curl handle object
#' @export
#' @examples # Redirect + cookies
#' res <- curl_fetch("http://httpbin.org/cookies/set?foo=123&bar=ftw")
curl_fetch <- function(url, handle = new_handle()){
  .Call(R_curl_fetch_memory, url, handle)
}
