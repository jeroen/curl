#' @rdname utilities
#' @useDynLib curl R_curl_version
#' @export
curl_version <- function(){
  .Call(R_curl_version);
}

#' @rdname utilities
#' @useDynLib curl R_curl_options
#' @export
curl_options <- function(){
  .Call(R_curl_options);
}
