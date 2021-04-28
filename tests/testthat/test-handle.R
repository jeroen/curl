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
  if(curl::curl_version()$version == "7.62.0") Sys.sleep(1) #workaround for curl bug #3351
  expect_null(jsonlite::fromJSON(rawToChar(curl_fetch_memory(httpbin("cookies"), handle = h)$content))$cookies$bar)
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

test_that("setting request headers", {
  h <- new_handle()
  expect_length(curl:::handle_getheaders(h), 0)
  handle_setheaders(h, foo = 'bar', baz = 'bak')
  expect_equal(curl:::handle_getheaders(h), c("foo: bar", "baz: bak", "Expect: "))
  handle_setheaders(h, foo = '123')
  expect_equal(curl:::handle_getheaders(h), c("foo: 123",  "Expect: "))
  handle_setopt(h, httpheader = c("test: blabla", "foobar: 123"))
  expect_equal(curl:::handle_getheaders(h), c("test: blabla", "foobar: 123"))
  handle_setheaders(h)
  expect_equal(curl:::handle_getheaders(h), c("Expect: "))
  handle_reset(h)
  expect_length(curl:::handle_getheaders(h), 0)
})

test_that("Custom vector options", {
  h <- new_handle()
  x <- c("foo@gmail.com", "bar@jkhk.nl")
  handle_setopt(h, mail_rcpt = x)
  expect_equal(curl:::handle_getcustom(h), x)

  # This leaks a bit
  x <- c(x, "jeroen@test.nl")
  handle_setopt(h, quote = c("bla"))
  handle_setopt(h, mail_rcpt = x)
  expect_equal(curl:::handle_getcustom(h), x)

  # Test free'ing
  handle_setopt(h, quote = NULL)
  handle_setopt(h, mail_rcpt = NULL)
  handle_reset(h)
  handle_setopt(h, quote = c("bla"))
})

test_that("Custom URL parser", {
  h <- new_handle(timeout = 1L)
  expect_error(curl_fetch_memory('https://httpbin.org/delay/10', handle = h), 'Timeout was reached: [httpbin.org] ', fixed = TRUE)

  h <- new_handle(timeout = 1L)
  expect_error(curl_fetch_memory('httpbin.org/delay/10', handle = h), 'Timeout was reached: [httpbin.org] ', fixed = TRUE)
})

test_that("Platform specific features", {
  if(.Platform$OS.type == 'windows'){
    ssl_version <- curl_version()$ssl_version
    if(get_windows_build() < 7600 || grepl("openssl", Sys.getenv('CURL_SSL_BACKEND'), TRUE)){
      expect_match(ssl_version, "OpenSSL.*\\(Schannel\\)")
    } else {
      expect_match(ssl_version, "\\(OpenSSL.*\\) Schannel")
    }
  } else if(!is.na(curl_options()['unix_socket_path'])){
    # This should simply not error
    expect_is(new_handle(UNIX_SOCKET_PATH = ""), "curl_handle")
  }
})

rm(h)
test_that("GC works", {
  gc()
  expect_equal(total_handles(), 0L)
})
