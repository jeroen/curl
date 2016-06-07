context("Certificate validation")

test_that("Invalid domain raises an error", {
  expect_equal(curl_fetch_memory("https://httpbin.org/get")$status, 200)
  fake_url <- paste0("https://", nslookup("httpbin.org"), "/get")
  expect_error(curl_fetch_memory(fake_url), "certificate")
  expect_equal(curl_fetch_memory(fake_url, handle = new_handle(ssl_verifyhost = FALSE))$status, 200)
})
