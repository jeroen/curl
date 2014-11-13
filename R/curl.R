#' @useDynLib curl R_curl_connection
curl <- function(url = "http://httpbin.org/get"){
  .Call(R_curl_connection, url)
}