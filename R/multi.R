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
#' When the request succeeded, the \code{done} callback gets triggerd with
#' the response data. The structure if this data is identical to \link{curl_fetch_memory}.
#' When the request fails, the \code{fail} callback is triggered with an error
#' message. Note that failure here means something went wrong in performing the
#' request such as a connection failure, it does not check the http status code.
#' Just like \link{curl_fetch_memory}, the user has to implement application logic.
#'
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
#' @rdname multi
#' @useDynLib curl R_multi_add
#' @param handle a curl \link{handle} with preconfigured \code{url} option.
#' @param done callback function for completed request. Single argument with
#' response data in same structure as \link{curl_fetch_memory}.
#' @param fail callback function called on failed request. Argument contains
#' error message.
#' @param pool a multi handle created by \link{new_pool}. Default uses a global pool.
#' @export
#' @examples h1 <- new_handle(url = "https://eu.httpbin.org/delay/3")
#' h2 <- new_handle(url = "https://eu.httpbin.org/post", postfields = "bla bla")
#' h3 <- new_handle(url = "https://urldoesnotexist.xyz")
#' multi_add(h1, done = print, fail = print)
#' multi_add(h2, done = print, fail = print)
#' multi_add(h3, done = print, fail = print)
#' multi_run(timeout = 2)
#' multi_run()
multi_add <- function(handle, done = NULL, fail = NULL, pool = NULL){
  if(is.null(pool))
    pool <- multi_default()
  stopifnot(inherits(handle, "curl_handle"))
  stopifnot(inherits(pool, "curl_multi"))
  stopifnot(is.null(done) || is.function(done))
  stopifnot(is.null(fail) || is.function(fail))
  .Call(R_multi_add, handle, done, fail, pool)
}

#' @param timeout max time in seconds to wait for results. Use \code{0} to poll for results without
#' waiting at all.
#' @export
#' @useDynLib curl R_multi_run
#' @rdname multi
multi_run <- function(timeout = Inf, pool = NULL){
  if(is.null(pool))
    pool <- multi_default()
  stopifnot(is.numeric(timeout))
  stopifnot(inherits(pool, "curl_multi"))
  .Call(R_multi_run, pool, timeout)
}

#' @param total_con max total concurrent connections.
#' @param host_con max concurrent connections per host.
#' @param multiplex enable HTTP/2 multiplexing if supported by host and client.
#' @export
#' @useDynLib curl R_multi_setopt
#' @rdname multi
multi_set <- function(total_con = 50, host_con = 6, multiplex = TRUE, pool = NULL){
  if(is.null(pool))
    pool <- multi_default()
  stopifnot(inherits(pool, "curl_multi"))
  stopifnot(is.numeric(total_con))
  stopifnot(is.numeric(host_con))
  stopifnot(is.logical(multiplex))
  .Call(R_multi_setopt, pool, total_con, host_con, multiplex)
}

#' @export
#' @useDynLib curl R_multi_list
#' @rdname multi
multi_list <- function(pool = NULL){
  if(is.null(pool))
    pool <- multi_default()
  stopifnot(inherits(pool, "curl_multi"))
  .Call(R_multi_list, pool)
}

#' @export
#' @useDynLib curl R_multi_cancel
#' @rdname multi
multi_cancel <- function(handle){
  stopifnot(inherits(handle, "curl_handle"))
  .Call(R_multi_cancel, handle)
}

#' @export
#' @useDynLib curl R_multi_new
#' @rdname multi
new_pool <- function(total_con = 100, host_con = 6, multiplex = TRUE){
  pool <- .Call(R_multi_new)
  multi_set(pool = pool, total_con = total_con, host_con = host_con, multiplex = multiplex)
}

multi_default <- local({
  global_multi_handle <- NULL
  function(){
    if(is.null(global_multi_handle)){
      global_multi_handle <<- new_pool()
    }
    stopifnot(inherits(global_multi_handle, "curl_multi"))
    return(global_multi_handle)
  }
})

#' @export
print.curl_multi <- function(x, ...){
  len <- length(multi_list(x))
  cat(sprintf("<curl multi-pool> (%d pending requests)\n", len))
}
