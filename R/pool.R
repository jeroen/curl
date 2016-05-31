#' Async multi download
#'
#' Performs multiple concurrent requests, possibly using HTTP/2 multiplexing.
#' Advanced use only!
#'
#' Schedule a series of requests to be executed simultaneously. Results are
#' available via callback functions, or at the end when all requests have
#' completed.
#'
#' It is important that a single handle cannot be used for multiple simultaneous
#' requests. However it is possible (and often makes sense) to re-use a handle
#' within the callback function of a request using that same handle. It is up
#' to the user to make sure the same handle is not used in concurrent requests.
#'
#' Note that it is allowed to add additional requests to the pool while it is
#' running. This is often useful in situations where a given response requires
#' us to do a subsequent request.
#'
#' @param connections value for \code{CURLMOPT_MAX_TOTAL_CONNECTIONS}
#' @param multiplex uses HTTP/2 multiplexing when supported
#' @export
#' @rdname pools
#' @name async
pool_new <- function(connections = 6, multiplex = FALSE){

}

#' @param handle a prepared handle
#' @param complete callback on complete
#' @param error callback on error
#' @export
#' @rdname pools
pool_add <- function(pool, handle, complete = identity, error = identity){

}

#' @param pool a new handle pool
#' @param timeout max seconds the pool is allowed to run. Use \code{0} to run pool until
#' all requests have completed.
#' @param verbose prints requests status messages to terminal
#' @export
#' @rdname pools
pool_run <- function(pool, timeout = 0, verbose = FALSE){

}
