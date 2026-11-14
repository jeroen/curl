test_that("curl_connect + curl_send/curl_recv exchange bytes over TLS", {
  skip_if_offline()
  h <- curl_connect("imaps://imap.gmail.com")
  expect_equal(curl_send(h, "A1 CAPABILITY\r\n"), nchar("A1 CAPABILITY\r\n"))
  out <- character(0)
  for (i in 1:5) {
    r <- curl_recv(h, timeout = 5000)
    if (length(r)) out <- c(out, rawToChar(r))
    if (any(grepl("A1 OK", out))) break
  }
  expect_true(any(grepl("CAPABILITY IMAP4rev1", out)))
  expect_equal(curl_send(h, "A2 LOGOUT\r\n"), nchar("A2 LOGOUT\r\n"))
})
