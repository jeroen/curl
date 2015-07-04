context("Certificate validation")

test_that("Perform", {
  if (!grepl("mingw", R.Version()$platform)) {
    expect_equal(curl_fetch_memory("https://httpbin.org/get")$status, 200)
    fake_url <- paste0("https://", utils::nsl("httpbin.org"), "/get")
    expect_error(curl_fetch_memory(fake_url), "certificate")
    expect_equal(curl_fetch_memory(fake_url, handle = new_handle(ssl_verifyhost = FALSE))$status, 200)
  }
})
