#' Fetch the contents of a URL.
#'
#' @param url A character string naming the URL of a resource to be downloaded.
#' @param handle a curl handle object
#' @export
#' @useDynLib curl R_curl_fetch_memory
#' @examples
#' # Redirect + cookies
#' res <- curl_fetch_memory("http://httpbin.org/cookies/set?foo=123&bar=ftw")
#' res$content
#'
#' # Save to disk
#' res <- curl_fetch_disk("http://httpbin.org/stream/10", tempfile())
#' res$content
#' readLines(res$content)
curl_fetch_memory <- function(url, handle = new_handle()){
  .Call(R_curl_fetch_memory, url, handle)
}

#' @export
#' @param param Path to save results
#' @rdname curl_fetch_memory
#' @useDynLib curl R_curl_fetch_disk
curl_fetch_disk <- function(url, path, handle = new_handle()){
  path <- normalizePath(path, mustWork = FALSE)
  .Call(R_curl_fetch_disk, url, handle, path, "wb")
}
