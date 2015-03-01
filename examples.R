library(curl)
h <- new_handle()
handle_setopt(h, COPYPOSTFIELDS = "mooo\nmooooo");
handle_setheader(h,
  "Content-Type" = "text/moo",
  "Cache-Control" = "no-cache",
  "User-Agent" = "A cow"
)
req = curl_perform("http://httpbin.org/post", handle = h)
cat(rawToChar(req$content))


# Posting JSON
library(jsonlite)
h <- new_handle()
handle_setopt(h, COPYPOSTFIELDS = toJSON(mtcars));
handle_setheader(h, "Content-Type" = "application/json")
req = curl_perform("http://httpbin.org/post", handle = h)
output <- fromJSON(rawToChar(req$content))
is.data.frame(output$json)



