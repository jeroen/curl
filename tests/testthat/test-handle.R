context("Reusable handle")

h <- new_handle()

test_that("Perform", {
  expect_equal(curl_fetch_memory("http://httpbin.org/get", handle = h)$status, 200)
  expect_equal(curl_fetch_memory("http://httpbin.org/cookies", handle = h)$status, 200)
  expect_equal(curl_fetch_memory("http://httpbin.org/status/418", handle = h)$status, 418)
})

test_that("Redirect", {
  expect_equal(curl_fetch_memory("http://httpbin.org/redirect/6", handle = h)$status, 200)
  expect_equal(curl_fetch_memory("http://httpbin.org/relative-redirect/6", handle = h)$status, 200)
  expect_equal(curl_fetch_memory("http://httpbin.org/absolute-redirect/6", handle = h)$status, 200)
})

test_that("Cookies", {
  expect_equal(curl_fetch_memory("http://httpbin.org/cookies/set?foo=123&bar=456", handle = h)$status, 200)
  expect_equal(jsonlite::fromJSON(rawToChar(curl_fetch_memory("http://httpbin.org/cookies", handle = h)$content))$cookies$bar, "456")
  expect_equal(curl_fetch_memory("http://httpbin.org/cookies/delete?bar", handle = h)$status, 200)
  expect_equal(jsonlite::fromJSON(rawToChar(curl_fetch_memory("http://httpbin.org/cookies", handle = h)$content))$cookies$bar, NULL)
})

test_that("Compression", {
  expect_equal(jsonlite::fromJSON(readLines(curl("http://httpbin.org/deflate", handle = h)))$deflate, TRUE)
  expect_equal(jsonlite::fromJSON(rawToChar(curl_fetch_memory("http://httpbin.org/deflate", handle = h)$content))$deflate, TRUE)
  expect_equal(jsonlite::fromJSON(readLines(curl("http://httpbin.org/gzip", handle = h)))$gzipped, TRUE)
  expect_equal(jsonlite::fromJSON(rawToChar(curl_fetch_memory("http://httpbin.org/gzip", handle = h)$content))$gzipped, TRUE)
  closeAllConnections()
})

test_that("Connection interface", {
  expect_equal(jsonlite::fromJSON(curl("http://httpbin.org/get?test=blabla", handle = h))$args$test, "blabla")
  expect_equal(jsonlite::fromJSON(curl("http://httpbin.org/cookies", handle = h))$cookies$foo, "123")
  expect_error(readLines(curl("http://httpbin.org/status/418")))
  closeAllConnections()
})

test_that("Opening and closing a connection",{
  # Create connection
  con <- curl("http://httpbin.org/cookies", handle = h)

  # Handle is still usable
  expect_equal(curl_fetch_memory("http://httpbin.org/get", handle = h)$status, 200)

  # Opening the connection locks the handle
  open(con)

  # Recent versions of libcurl will raise an error
  #if(compareVersion(curl_version()$version, "7.37") > 0){
  #  expect_error(curl_fetch_memory("http://httpbin.org/get", handle = h))
  #}

  expect_equal(jsonlite::fromJSON(readLines(con))$cookies$foo, "123")

  # After closing it is free again
  close(con)
  expect_equal(curl_fetch_memory("http://httpbin.org/get", handle = h)$status, 200)

  # Removing the connection also unlocks the handle
  con <- curl("http://httpbin.org/cookies", "rb", handle = h)

  # Recent versions of libcurl will raise an error
  #if(compareVersion(curl_version()$version, "7.37") > 0){
  #  expect_error(curl_fetch_memory("http://httpbin.org/get", handle = h))
  #}

  rm(con)
  suppressWarnings(gc())
  expect_equal(curl_fetch_memory("http://httpbin.org/get", handle = h)$status, 200)
})

test_that("Downloading to a file", {
  tmp <- tempfile()
  expect_error(curl_download("http://httpbin.org/status/418", tmp, handle = h))

  curl_download("http://httpbin.org/get?test=boeboe", tmp, handle = h)
  expect_equal(jsonlite::fromJSON(tmp)$args$test, "boeboe")

  curl_download("http://httpbin.org/cookies", tmp, handle = h)
  expect_equal(jsonlite::fromJSON(tmp)$cookies$foo, "123")
  suppressWarnings(gc())
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
  suppressWarnings(gc())
})
