#' Mocking HTTP requests
#' @export
#' @param on (logical) turn mocking on with `TRUE` or turn off with `FALSE`.
#' By default is `FALSE`
mock <- function(on = TRUE) {
  if (!requireNamespace("webmockr", quietly = TRUE))
      stop("Please install 'webmockr'")
  curl_mock_env$mock <- on
}

#' @keywords internal
#' @examples
#' h <- new_handle()
#' handle_setopt(h, customrequest = "PUT")
#' handle_setform(h, a = "1", b = "2")
#' handle_setheaders(h, a = 'b')
#' handle_setheaders(h, foo = 'bar')
#' url = "https://httpbin.org/put"
#' 
#' h <- new_handle()
#' handle_setopt(h, COPYPOSTFIELDS = jsonlite::toJSON(mtcars))
#' handle_setheaders(h, "Content-Type" = "application/json")
#' 
#' h <- new_handle()
#' handle_setopt(h, COPYPOSTFIELDS = jsonlite::toJSON(mtcars))
#' handle_setheaders(h, "Content-Type" = "application/json")
#' handle_setopt(h, userpwd = "foo:bar")
#' handle_setopt(h, httpauth = 1)
mock_req <- function(url, h, called) {
  # cat(paste0("mocking? ", curl_mock_env$mock), "\n")
  # if (curl_mock_env$mock) {
    if (!requireNamespace("webmockr", quietly = TRUE))
      stop("Please install 'webmockr'")

    # cat("calling curl_echo from inside mock_req", "\n")
    # run curl_echo to get components
    res <- curl_echo(h)
    # cat(rawToChar(res$body))
    req <- list(url = url, handle = h, called = called)
    req$method <- res$request_method
    if (!is.null(res$http_authorization)) {
      req$auth <- 
        as.list(stats::setNames(
          strsplit(res$http_authorization, "\\s")[[1]], 
          c('type', 'user_pwd')
        ))
    }
    req$headers <- res[grepl("^http_", names(res))]
    req$headers$http_host <- NULL 
    req$headers$http_authorization <- NULL
    req$headers$server_name <- NULL
    req$headers$httpuv <- NULL
    if (!is.null(res$body)) req$body <- rawToChar(res$body)

    # handle request
    # cat("calling adap$handle_request from inside mock_req", "\n")
    adap <- webmockr::CurlAdapter$new()
    adap$handle_request(req)
  # }
}

within_echo <- function() grepl("curl_echo", deparse(match.call()))
