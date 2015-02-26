library(curl)
h <- new_handle()
tmp <- tempfile()
con <- curl("http://httpbin.org/get", handle = h)
readLines(con)
gc()
readLines(con)
curl_download("http://httpbin.org/get", tmp, handle = h)
readLines(tmp)
unlink(tmp)
curl_download("http://httpbin.org/cookies", tmp, handle = h)
readLines(tmp)
unlink(tmp)
curl_perform("http://httpbin.org/get", handle = h)
curl_perform("http://httpbin.org/cookies", handle = h)
curl_download("http://httpbin.org/get", tmp, handle = h)
readLines(tmp)
gc()
con <- curl("http://httpbin.org/get", handle = h)
gc()
readLines(con)
readLines(con)

close(con)
curl_perform("http://httpbin.org/get", handle = h)

con <- curl("http://httpbin.org/get", handle = h)
open(con)
readLines(con)
close(con)
curl_perform("http://httpbin.org/get", handle = h)

# What you should NOT do
# You cannot open a connection on a handle and then use the handle elsewhere
h <- new_handle()
con <- curl("http://httpbin.org/get", handle = h)
open(con)

#This will error
curl_perform("http://httpbin.org/get", handle = h)

# But it will work after you close the connection
close(con)
curl_perform("http://httpbin.org/get", handle = h)




