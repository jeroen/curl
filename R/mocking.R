#' Mocking HTTP requests
#' @export
#' @param on (logical) turn mocking on with `TRUE` or turn off with `FALSE`.
#' By default is `FALSE`
mock <- function(on = TRUE) {
  check_for_package("webmockr")
  curl_opts$mock <- on
}

# internal method
mock_req <- function(url, handle) {
  if (curl_opts$mock) {
    if (!requireNamespace("webmockr", quietly = TRUE)) 
      stop("Please install 'webmockr'")
    adap <- webmockr::CurlAdapter$new()
    return(adap$handle_request(url, handle))
  }
}
