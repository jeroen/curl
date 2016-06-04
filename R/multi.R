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
#' @name multi
#' @param handle a prepared handle
#' @param complete callback on complete
#' @param error callback on error
#' @export
#' @useDynLib curl R_multi_add
#' @rdname multi
#' @examples pool <- multi_new()
#' h1 <- new_handle(url = "https://httpbin.org/get")
#' h2 <- new_handle(url = "https://httpbin.org/post", postfields = "bla bla")
#' h3 <- new_handle(url = "https://urldoesnotexist.xyz")
#' multi_add(pool, h1, function(res){print(res)})
#' multi_add(pool, h2, function(res){print(res)})
#' multi_add(pool, h3)
#' multi_run(pool)
multi_add <- function(handle, complete = identity, error = identity){
  stopifnot(inherits(handle, "curl_handle"))
  stopifnot(is.function(complete))
  stopifnot(is.function(error))
  .Call(R_multi_add, handle, complete, error)
}

#' @export
#' @useDynLib curl R_multi_remove
#' @rdname multi
multi_remove <- function(handle){
  stopifnot(inherits(handle, "curl_handle"))
  .Call(R_multi_remove, handle)
}

#' @param timeout max seconds the pool is allowed to run. Use \code{0} to run pool until
#' all requests have completed.
#' @param multiplex enable HTTP/2 multiplexing if supported
#' @param connections max number of concurrent connections
#' @param verbose prints requests status messages to terminal
#' @export
#' @useDynLib curl R_multi_run
#' @rdname multi
multi_run <- function(multiplex = TRUE, connections = 6, timeout = -1, verbose = FALSE){
  .Call(R_multi_run, multiplex, connections, timeout)
}
