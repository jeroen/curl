#' Async Multi Download
#'
#' AJAX style concurrent requests, possibly using HTTP/2 multiplexing.
#' Results are only available via callback functions. Advanced use only!
#'
#' Requests are created in the usual way using a curl \link{handle} and added
#' to the scheduler with \link{multi_add}. This function returns immediately
#' and does not perform the request yet. The user needs to call \link{multi_run}
#' which performs all scheduled requests concurrently. It returns when all
#' requests have completed, or case of a \code{timeout} or \code{SIGINT} (e.g.
#' if the user presses \code{ESC} or \code{CTRL+C} in the console). In case of
#' the latter, simply call \link{multi_run} again to resume pending requests.
#'
#' When the request succeeded, the \code{complete} callback gets triggerd with
#' the response data. The structure if this data is identical to \link{curl_fetch_memory}.
#' When the request fails, the \code{error} callback is triggered with an error
#' message. Note that failure here means something went wrong in performing the
#' request such as a connection failure, not that the HTTP returned 200. Similar
#' to  \link{curl_fetch_memory}, the user has to implement application logic.
#' Raising an error within a callback function stops execution of that function
#' but does not affect other requests.
#'
#' A single handle cannot be used for multiple simultaneous requests. However
#' it is possible to add new requests to a pool while it is running, so you
#' can re-use a handle within the callback of a request from that same handle.
#' It is up to the user to make sure the same handle is not used in concurrent
#' requests.
#'
#' The \link{multi_cancel} function can be used to cancel a pending request.
#' It has no effect if the request was already completed or canceled.
#'
#' @name multi
#' @param handle a curl \link{handle} with \code{url} option already set.
#' @param complete callback function for successful request. Single argument with
#' response data in same structure as \link{curl_fetch_memory}.
#' @param error callback function called on failed request. Argument contains
#' error message.
#' @export
#' @useDynLib curl R_multi_add
#' @rdname multi
#' @examples h1 <- new_handle(url = "https://httpbin.org/delay/3")
#' h2 <- new_handle(url = "https://httpbin.org/post", postfields = "bla bla")
#' h3 <- new_handle(url = "https://urldoesnotexist.xyz")
#' multi_add(h1, complete = print, error = print)
#' multi_add(h2, complete = print, error = print)
#' multi_add(h3, complete = print, error = print)
#' multi_run(timeout = 2)
#' multi_run()
multi_add <- function(handle, complete = NULL, error = NULL){
  stopifnot(inherits(handle, "curl_handle"))
  stopifnot(is.null(complete) || is.function(complete))
  stopifnot(is.null(error) || is.function(error))
  .Call(R_multi_add, handle, complete, error)
}

#' @param timeout max time in seconds to wait for results. Use \code{0} to poll for results without
#' waiting at all.
#' @param total_connetions max total concurrent connections.
#' @param host_connections max concurrent connections per host.
#' @param multiplex enable HTTP/2 multiplexing if supported by host and client.
#' @export
#' @useDynLib curl R_multi_run
#' @rdname multi
multi_run <- function(timeout = Inf, total_connetions = 100, host_connections = 6, multiplex = TRUE){
  .Call(R_multi_run, timeout, total_connetions, host_connections, multiplex)
}

#' @export
#' @useDynLib curl R_multi_cancel
#' @rdname multi
multi_cancel <- function(handle){
  stopifnot(inherits(handle, "curl_handle"))
  .Call(R_multi_cancel, handle)
}

