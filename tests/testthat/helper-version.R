options(curl_interrupt = TRUE)
cat("This is libcurl version", curl_version()$version, "with", curl_version()$ssl_version, "\n")

# Try to load test server
testserver <- "https://http2bin.org"
req <- curl_fetch_memory("http://jeroenooms.github.io/curl/httpbin")
if(req$status == 200){
  testserver <- paste0(sub("[ \t\r\n]+", "", rawToChar(req$content)), "/")
}
cat("Using test server:", testserver, "\n")

httpbin <- function(path){
  paste0(testserver, sub("^/", "", path))
}
