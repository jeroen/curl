context("Authentication")

h <- new_handle()

test_that("Authentication", {
  expect_equal(curl_perform("http://httpbin.org/basic-auth/jerry/secret", handle = h)$status, 401)
  expect_equal(curl_perform("http://httpbin.org/hidden-basic-auth/jerry/secret", handle = h)$status, 404)
  expect_equal(curl_perform("http://httpbin.org/digest-auth/auth/jerry/secret", handle = h)$status, 401)

  handle_setopt(h, username = "jerry", password = "secret")
  expect_equal(curl_perform("http://httpbin.org/basic-auth/jerry/secret", handle = h)$status, 200)
  expect_equal(curl_perform("http://httpbin.org/hidden-basic-auth/jerry/secret", handle = h)$status, 200)
  expect_equal(curl_perform("http://httpbin.org/digest-auth/auth/jerry/secret", handle = h)$status, 401)

  handle_reset(h)
  handle_setopt(h, userpwd = "jerry:secret")
  expect_equal(curl_perform("http://httpbin.org/basic-auth/jerry/secret", handle = h)$status, 200)
  expect_equal(curl_perform("http://httpbin.org/hidden-basic-auth/jerry/secret", handle = h)$status, 200)
  expect_equal(curl_perform("http://httpbin.org/digest-auth/auth/jerry/secret", handle = h)$status, 401)

  handle_setopt(h, httpauth = 2)
  expect_equal(curl_perform("http://httpbin.org/digest-auth/auth/jerry/secret", handle = h)$status, 200)

})
