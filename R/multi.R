#' Async multi download
#'
#' Performs multiple concurrent requests, possibly using HTTP/2 multiplexing.
#' Advanced use only!
#'
#' Schedule a series of requests to be executed simultaneously. Results are
#' available via callback functions, or at the end when all requests have
#' completed.
#'
#' A single handle cannot be used for multiple simultaneous requests. However
#' it is possible (and often sensible) to re-use a handle within the callback
#' of a request from that same handle. It is up to the user to make sure the
#' same handle is not used in concurrent requests.
#'
#' It is possible to add new requests to a pool while it is running. This can
#' be useful in situations where a given response requires subsequent requests
#' to retrieve additional information.
#'
#' @param connections value for \code{CURLMOPT_MAX_TOTAL_CONNECTIONS}
#' @param multiplex uses HTTP/2 multiplexing when supported
#' @export
#' @rdname multi
#' @name multi
#' @examples pool <- multi_new()
#' h1 <- new_handle(url = "https://httpbin.org/get")
#' h2 <- new_handle(url = "https://httpbin.org/post", httppost = TRUE)
#' multi_add(pool, )
#' multi_add(pool, )
multi_new <- function(connections = 6, multiplex = FALSE){

}

#' @param handle a prepared handle
#' @param complete callback on complete
#' @param error callback on error
#' @export
#' @rdname multi
multi_add <- function(multi, handle, complete = identity, error = identity){

}

#' @param multi a new handle pool
#' @param timeout max seconds the pool is allowed to run. Use \code{0} to run pool until
#' all requests have completed.
#' @param verbose prints requests status messages to terminal
#' @export
#' @rdname multi
multi_run <- function(multi, timeout = 0, verbose = FALSE){

}
