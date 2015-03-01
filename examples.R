library(curl)
h <- new_handle()
handle_setopt(h, COPYPOSTFIELDS = "mooo\nmooooo");
handle_setheader(h,
  "Content-Type" = "text/moo",
  "Cache-Control" = "no-cache",
  "User-Agent" = "A cow"
)

# Using perform interface
req = curl_perform("http://httpbin.org/post", handle = h)
cat(rawToChar(req$content))

# Or with connection interface
cat(readLines(curl("http://httpbin.org/post", handle = h)), sep = "\n")

# Posting JSON
library(jsonlite)
h <- new_handle()
handle_setopt(h, COPYPOSTFIELDS = toJSON(mtcars));
handle_setheader(h, "Content-Type" = "application/json")
req = curl_perform("http://httpbin.org/post", handle = h)
output <- fromJSON(rawToChar(req$content))

# Note that httpbin reodered columns alphabetically
print(output$json)

# Posting multipart
library(curl)
h <- new_handle()
handle_setform(h, foo = "blabla", bar = charToRaw("boeboe"))
req = curl_perform("http://httpbin.org/post", handle = h)
cat(rawToChar(req$content))
