context("Cookies")

h <- new_handle()

test_that("No cookies", {
  cookies <- handle_cookies(h);
  expect_is(cookies, "data.frame")
  expect_equal(nrow(cookies), 0)
})

test_that("Add some cookies", {
  req <- curl_fetch_memory("http://httpbin.org/cookies/set?foo=123&bar=ftw", handle = h)
  cookies <- handle_cookies(h);
  expect_is(cookies, "data.frame")
  expect_equal(nrow(cookies), 2)
  expect_equal(cookies$name, c("foo","bar"))
  expect_equal(cookies$value, c("123","ftw"))
  expect_true(all(cookies$expiration == Inf))
})

test_that("Delete a cookie", {
  req <- curl_fetch_memory("http://httpbin.org/cookies/delete?foo", handle = h)
  cookies <- handle_cookies(h)
  foo <- subset(cookies, name == "foo")
  bar <- subset(cookies, name == "bar")
  expect_true(foo$expiration < Sys.time())
  expect_true(bar$expiration > Sys.time())
  expect_true(is.na(foo$value))
  expect_equal(bar$value, "ftw")
})

test_that("Overwrite a cookie", {
  req <- curl_fetch_memory("http://httpbin.org/cookies/set?foo=888&bar=999", handle = h)
  cookies <- handle_cookies(h)
  foo <- subset(cookies, name == "foo")
  bar <- subset(cookies, name == "bar")
  expect_equal(foo$value, "888")
  expect_equal(bar$value, "999")
  expect_true(all(cookies$expiration == Inf))
})
