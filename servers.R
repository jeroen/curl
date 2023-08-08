# Temp workaround for bug with libcurl 8.0.0
tryCatch({
  lines <- readLines('test-multi.R')
  lines <- sub("expect_length(c(fdset", "#expect_length(c(fdset", lines, fixed = TRUE)
  writeLines(lines, 'test-multi.R')
}, error = function(e){})

#NB: hb.cran.dev uses cloudflare proxy which tags on some headers
c("https://hb.r-universe.dev",
  "http://hb.opencpu.org", "https://nghttp2.org/httpbin", "https://eu.httpbin.org",
  "https://httpbin.org", "http://httpbin.org")
