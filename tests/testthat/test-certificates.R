context("Certificate validation")

test_that("Invalid domain raises an error", {
  expect_is(curl_fetch_memory(httpbin("get"))$status, "integer")
  fake_url <- paste0("https://", nslookup("httpbin.org"), "/get")
  expect_error(curl_fetch_memory(fake_url), "certificate")
  expect_is(curl_fetch_memory(fake_url, handle = new_handle(ssl_verifyhost = FALSE))$status, "integer")
})

test_that("GC works", {
  gc()
  expect_equal(total_handles(), 0L)
})
