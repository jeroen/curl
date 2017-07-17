context("Certificate validation")

test_that("CloudFlare / LetsEncrypt certs", {
  if(is.numeric(get_windows_build()))
    skip_if_not(get_windows_build() >= 7600, "TLS 1.2 requires at least Windows 7 / Windows Server 2008 R2")
  expect_equal(curl_fetch_memory('https://www.opencpu.org')$status_code, 200)
  expect_equal(curl_fetch_memory('https://rud.is')$status_code, 200)
})

test_that("Invalid domain raises an error", {
  ipaddr <- nslookup("www.google.com", ipv4_only = TRUE)
  fake_url <- paste0("https://", ipaddr)
  expect_error(curl_fetch_memory(fake_url), "certificate")
  expect_is(curl_fetch_memory(fake_url, handle = new_handle(ssl_verifyhost = FALSE))$status, "integer")
})

test_that("GC works", {
  gc()
  expect_equal(total_handles(), 0L)
})
