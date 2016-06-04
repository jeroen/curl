#' Async multi download
#'
#' Performs multiple concurrent requests, possibly using HTTP/2 multiplexing.
#' Results are only available via callback functions. Advanced use only!
#'
#' A single handle cannot be used for multiple simultaneous requests. However
#' it is possible to add new requests to a pool while it is running, so you
#' can re-use a handle within the callback of a request from that same handle.
#' It is up to the user to make sure the same handle is not used in concurrent
#' requests.
#'
#' @name multi
#' @param handle a prepared handle
#' @param complete callback on complete
#' @param error callback on error
#' @export
#' @useDynLib curl R_multi_add
#' @rdname multi
#' @examples h1 <- new_handle(url = "https://httpbin.org/delay/5")
#' h2 <- new_handle(url = "https://httpbin.org/post", postfields = "bla bla")
#' h3 <- new_handle(url = "https://urldoesnotexist.xyz")
#' multi_add(h1, function(res){print(res)})
#' multi_add(h2, function(res){print(res)})
#' multi_add(h3)
#' multi_run(timeout = 3)
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

#' @param timeout max seconds the pool is allowed to run. Use \code{0} to just poll for
#' results without waiting. Use \code{-1} to wait untill all requests have completed.
#' @param multiplex enable HTTP/2 multiplexing if supported
#' @param connections max number of concurrent connections
#' @export
#' @useDynLib curl R_multi_run
#' @rdname multi
multi_run <- function(multiplex = TRUE, connections = 6, timeout = -1){
  .Call(R_multi_run, multiplex, connections, timeout)
}
