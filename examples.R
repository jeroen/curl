library(curl)
h <- new_handle()
handle_setopt(h, COPYPOSTFIELDS = "mooo\nmooooo");
handle_setheader(h,
  "Content-Type" = "text/moo",
  "Cache-Control" = "no-cache",
  "User-Agent" = "A cow"
)

# Using perform interface
req <- curl_perform("http://httpbin.org/post", handle = h)
cat(rawToChar(req$content))

# Or with connection interface
cat(readLines(curl("http://httpbin.org/post", handle = h)), sep = "\n")

# Reset headers
handle_setheader(h, "User-Agent" = "Not a cow")
req <- curl_perform("http://httpbin.org/post", handle = h)
cat(rawToChar(req$content))

# Reset all fields
handle_reset(h)
req <- curl_perform("http://httpbin.org/get", handle = h)
cat(rawToChar(req$content))

# Posting JSON
library(jsonlite)
h <- new_handle()
handle_setopt(h, COPYPOSTFIELDS = toJSON(mtcars));
handle_setheader(h, "Content-Type" = "application/json")
req <- curl_perform("http://httpbin.org/post", handle = h)
output <- fromJSON(rawToChar(req$content))

# Note that httpbin reodered columns alphabetically
stopifnot(is.data.frame(output$json))
rm(h)
gc()

# Posting multipart
library(curl)
h <- new_handle()
handle_setform(h,
  foo = "blabla",
  bar = charToRaw("boeboe"),
  description = form_file(system.file("DESCRIPTION")),
  logo = form_file(file.path(Sys.getenv("R_DOC_DIR"), "html/logo.jpg"), "image/jpeg")
)
req <- curl_perform("http://httpbin.org/post", handle = h)
cat(rawToChar(req$content))

# Reset again
handle_reset(h)
req <- curl_perform("http://httpbin.org/get", handle = h)
cat(rawToChar(req$content))

