# Some of these unit tests fail if you reuse the handle. Don't know why. Maybe cache related.
context("Authentication")

test_that("Permission denied", {
  h <- new_handle()
  expect_equal(curl_fetch_memory("http://httpbin.org/basic-auth/jerry/secret", handle = h)$status, 401)
  expect_equal(curl_fetch_memory("http://httpbin.org/hidden-basic-auth/jerry/secret", handle = h)$status, 404)
  expect_equal(curl_fetch_memory("http://httpbin.org/digest-auth/auth/jerry/secret", handle = h)$status, 401)
})

test_that("Auth userpwd", {
  h <- new_handle()
  handle_setopt(h, userpwd = "jerry:secret")
  expect_equal(curl_fetch_memory("http://httpbin.org/basic-auth/jerry/secret", handle = h)$status, 200)
  expect_equal(curl_fetch_memory("http://httpbin.org/hidden-basic-auth/jerry/secret", handle = h)$status, 200)
  expect_equal(curl_fetch_memory("http://httpbin.org/digest-auth/auth/jerry/secret", handle = h)$status, 200)
})

test_that("Auth username and password", {
  h <- new_handle()
  handle_setopt(h, username = "jerry", password = "secret")
  expect_equal(curl_fetch_memory("http://httpbin.org/basic-auth/jerry/secret", handle = h)$status, 200)
  expect_equal(curl_fetch_memory("http://httpbin.org/hidden-basic-auth/jerry/secret", handle = h)$status, 200)
  expect_equal(curl_fetch_memory("http://httpbin.org/digest-auth/auth/jerry/secret", handle = h)$status, 200)
})
