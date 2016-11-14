context("Reusable handle")

h <- new_handle()

test_that("Perform", {
  expect_equal(curl_fetch_memory(httpbin("get"), handle = h)$status, 200)
  expect_equal(curl_fetch_memory(httpbin("cookies"), handle = h)$status, 200)
  expect_equal(curl_fetch_memory(httpbin("status/418"), handle = h)$status, 418)
})

test_that("Redirect", {
  expect_equal(curl_fetch_memory(httpbin("redirect/6"), handle = h)$status, 200)
  expect_equal(curl_fetch_memory(httpbin("relative-redirect/6"), handle = h)$status, 200)
  expect_equal(curl_fetch_memory(httpbin("absolute-redirect/6"), handle = h)$status, 200)
})

test_that("Cookies", {
  expect_equal(curl_fetch_memory(httpbin("cookies/set?foo=123&bar=456"), handle = h)$status, 200)
  expect_equal(jsonlite::fromJSON(rawToChar(curl_fetch_memory(httpbin("cookies"), handle = h)$content))$cookies$bar, "456")
  expect_equal(curl_fetch_memory(httpbin("cookies/delete?bar"), handle = h)$status, 200)
  expect_equal(jsonlite::fromJSON(rawToChar(curl_fetch_memory(httpbin("cookies"), handle = h)$content))$cookies$bar, NULL)
})

test_that("Keep-Alive", {
  # Connection to httpbin already set in previous tests. Subsequent requests
  # should reuse the connection.
  # Capture the verbose curl output to look for the connection reuse message
  h <- handle_setopt(h, verbose=TRUE,
    debugfunction=function(type, msg) cat(readBin(msg, character())))
  req <- capture.output(curl_fetch_memory(httpbin("get"), handle=h))
  expect_true(any(grepl("existing connection", req)))
  handle_setopt(h, verbose=FALSE)
})

test_that("Compression and destroying connection", {
  con <- curl(httpbin("deflate"), handle = h)
  expect_equal(jsonlite::fromJSON(readLines(con))$deflate, TRUE)
  expect_false(isOpen(con))
  close(con) #destroy

  expect_equal(jsonlite::fromJSON(rawToChar(curl_fetch_memory(httpbin("deflate"), handle = h)$content))$deflate, TRUE)

  con <- curl(httpbin("gzip"), handle = h)
  expect_equal(jsonlite::fromJSON(readLines(con))$gzipped, TRUE)
  expect_false(isOpen(con))
  close(con) #destroy

  expect_equal(jsonlite::fromJSON(rawToChar(curl_fetch_memory(httpbin("gzip"), handle = h)$content))$gzipped, TRUE)
})

test_that("Connection interface", {
  # note: jsonlite automatically destroys auto-opened connection
  con <- curl(httpbin("get?test=blabla"), handle = h)
  expect_equal(jsonlite::fromJSON(con)$args$test, "blabla")
  con <- curl(httpbin("cookies"), handle = h)
  expect_equal(jsonlite::fromJSON(con)$cookies$foo, "123")

  # test error
  con <- curl(httpbin("status/418"))
  expect_error(readLines(con))
  close(con) #destroy
})

test_that("Opening and closing a connection",{
  # Create connection
  con <- curl(httpbin("cookies"), handle = h)

  # Handle is still usable
  expect_equal(curl_fetch_memory(httpbin("get"), handle = h)$status, 200)

  # Opening the connection locks the handle
  open(con)

  # Recent versions of libcurl will raise an error
  #if(compareVersion(curl_version()$version, "7.37") > 0){
  #  expect_error(curl_fetch_memory(httpbin("get", handle = h))
  #}

  expect_equal(jsonlite::fromJSON(readLines(con))$cookies$foo, "123")

  # After closing it is free again
  close(con)
  expect_equal(curl_fetch_memory(httpbin("get"), handle = h)$status, 200)

  # Removing the connection also unlocks the handle
  con <- curl(httpbin("cookies"), "rb", handle = h)

  # Recent versions of libcurl will raise an error
  #if(compareVersion(curl_version()$version, "7.37") > 0){
  #  expect_error(curl_fetch_memory(httpbin("get", handle = h))
  #}
  close(con)
  rm(con)
  expect_equal(curl_fetch_memory(httpbin("get"), handle = h)$status, 200)
})

test_that("Downloading to a file", {
  tmp <- tempfile()
  expect_error(curl_download(httpbin("status/418"), tmp, handle = h))

  curl_download(httpbin("get?test=boeboe"), tmp, handle = h)
  expect_equal(jsonlite::fromJSON(tmp)$args$test, "boeboe")

  curl_download(httpbin("cookies"), tmp, handle = h)
  expect_equal(jsonlite::fromJSON(tmp)$cookies$foo, "123")
})

test_that("handle_setopt validates options", {
  h <- new_handle()
  expect_identical(class(h), "curl_handle")
  expect_error(handle_setopt(h, invalid.option="foo"),
    "Unknown option: invalid.option")
  expect_error(handle_setopt(h, badopt1="foo", badopt2="bar"),
    "Unknown options: badopt1, badopt2")
  expect_identical(class(handle_setopt(h, username="foo")),
    "curl_handle") ## i.e. that's a valid option, so it succeeds
})

rm(h)
test_that("GC works", {
  gc()
  expect_equal(total_handles(), 0L)
})
