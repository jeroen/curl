#' Async Multi Download
#'
#' AJAX style concurrent requests, possibly using HTTP/2 multiplexing.
#' Results are only available via callback functions. Advanced use only!
#'
#' Requests are created in the usual way using a curl \link{handle} and added
#' to the scheduler with \link{multi_add}. This function returns immediately
#' and does not perform the request yet. After zero or more handles have been
#' added, the \code{multi_run} function performs all requests concurrently.
#' It returns when all requests have completed or when \code{timeout} has
#' passed, or when the user interrupts by pressing \code{ESC} or \code{CTRL+C}.
#' The \code{multi_run} function can be called to resume requests.
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
multi_add <- function(handle, complete = NULL, error = NULL){
  stopifnot(inherits(handle, "curl_handle"))
  stopifnot(is.null(complete) || is.function(complete))
  stopifnot(is.null(error) || is.function(error))
  .Call(R_multi_add, handle, complete, error)
}

#' @param timeout max time in seconds to wait for results. Use \code{0} to poll for results without
#' waiting at all.
#' @param total_connetions limit total concurrent connections
#' @param host_connections limit concurrent connections per host
#' @param multiplex enable HTTP/2 multiplexing if supported by host and client
#' @export
#' @useDynLib curl R_multi_run
#' @rdname multi
multi_run <- function(timeout = Inf, total_connetions = 100, host_connections = 6, multiplex = TRUE){
  .Call(R_multi_run, timeout, total_connetions, host_connections, multiplex)
}

#' @export
#' @useDynLib curl R_multi_remove
#' @rdname multi
multi_remove <- function(handle){
  stopifnot(inherits(handle, "curl_handle"))
  .Call(R_multi_remove, handle)
}

