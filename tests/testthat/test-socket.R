test_that("bidirectional socket connection over plain tcp", {
  skip_on_cran()
  skip_if(getRversion() < "4.0.0") # needs serverSocket()
  port <- find_port()
  server <- serverSocket(port)
  on.exit(close(server), add = TRUE)
  con <- curl(sprintf("telnet://127.0.0.1:%d", port), "r+b")
  on.exit(close(con), add = TRUE)
  peer <- socketAccept(server, blocking = TRUE, open = "r+b")

  # client to server
  writeBin(charToRaw("hello server\n"), con)
  expect_equal(readLines(peer, n = 1), "hello server")

  # server to client
  writeBin(charToRaw("hello client\n"), peer)
  expect_equal(rawToChar(readBin(con, raw(), 100)), "hello client\n")

  # peer disconnect is EOF
  close(peer)
  expect_equal(length(readBin(con, raw(), 100)), 0)
  expect_false(isIncomplete(con))
})

test_that("non-blocking reads on a socket connection", {
  skip_on_cran()
  skip_if(getRversion() < "4.0.0") # needs serverSocket()
  port <- find_port()
  server <- serverSocket(port)
  on.exit(close(server), add = TRUE)
  con <- curl(sprintf("telnet://127.0.0.1:%d", port))
  on.exit(close(con), add = TRUE)
  open(con, "r+b", blocking = FALSE)
  peer <- socketAccept(server, blocking = TRUE, open = "r+b")

  # no data available: return immediately with 0 bytes
  expect_equal(length(readBin(con, raw(), 100)), 0)
  expect_true(isIncomplete(con))

  # wait for data to arrive
  writeBin(charToRaw("ping\n"), peer)
  Sys.sleep(0.5)
  expect_equal(rawToChar(readBin(con, raw(), 100)), "ping\n")

  # peer disconnect is EOF
  close(peer)
  Sys.sleep(0.5)
  expect_equal(length(readBin(con, raw(), 100)), 0)
  expect_false(isIncomplete(con))
})

test_that("blocking reads on a socket connection honor options(timeout)", {
  skip_on_cran()
  skip_if(getRversion() < "4.0.0") # needs serverSocket()
  port <- find_port()
  server <- serverSocket(port)
  on.exit(close(server), add = TRUE)
  con <- curl(sprintf("telnet://127.0.0.1:%d", port), "r+b")
  on.exit(close(con), add = TRUE)
  peer <- socketAccept(server, blocking = TRUE, open = "r+b")
  on.exit(close(peer), add = TRUE)
  old <- options(timeout = 2)
  on.exit(options(old), add = TRUE)

  # blocking read with no incoming data returns empty after the timeout
  t0 <- Sys.time()
  out <- readBin(con, raw(), 100)
  elapsed <- as.numeric(Sys.time() - t0, units = "secs")
  expect_equal(length(out), 0)
  expect_true(isIncomplete(con)) # timeout, not EOF
  expect_gte(elapsed, 1.5)
  expect_lt(elapsed, 30)
})

test_that("socket connection speaks IMAP over TLS", {
  skip_on_cran()
  skip_if_offline("imap.gmail.com")
  con <- curl("imaps://imap.gmail.com", "r+")
  on.exit(close(con), add = TRUE)
  writeLines("A1 CAPABILITY", con, sep = "\r\n")
  out <- character()
  for(i in 1:50) {
    out <- c(out, readLines(con, n = 1))
    if(grepl("^A1 ", out[length(out)]))
      break
  }
  expect_true(any(grepl("CAPABILITY IMAP4rev1", out)))
  expect_true(grepl("^A1 OK", out[length(out)]))
  writeLines("A2 LOGOUT", con, sep = "\r\n")
  expect_match(readLines(con, n = 1), "BYE")
})
