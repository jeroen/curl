# This tests TLS 1.2
test_that("CloudFlare / LetsEncrypt certs", {
  expect_equal(curl_fetch_memory('https://www.opencpu.org')$status_code, 200)
  expect_equal(curl_fetch_memory('https://letsencrypt.org')$status_code, 200)

  # Test HTTP -> HTTPS (TLS 1.2) redirection
  expect_equal(curl_fetch_memory('http://curl.se')$status_code, 200)
})

test_that("Invalid domain raises an error", {
  ipaddr <- nslookup("www.r-project.org", ipv4_only = TRUE)
  h <- new_handle(resolve = paste0("fakehostname:443:", ipaddr))
  expect_error(curl_fetch_memory("https://fakehostname", handle = h), "certificate", class = "curl_error_peer_failed_verification")
  handle_setopt(h, ssl_verifyhost = FALSE, ssl_verifypeer = FALSE)
  expect_is(curl_fetch_memory("https://fakehostname", handle = h)$status, "integer")
})

test_that("GC works", {
  gc()
  expect_equal(total_handles(), 0L)
})
