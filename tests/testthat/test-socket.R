test_that("bidirectional socket connection over plain tcp", {
  skip_on_cran()
  skip_if(getRversion() < "4.0.0") # needs serverSocket()
  port <- find_port()
  server <- serverSocket(port)
  on.exit(close(server), add = TRUE)

  # socket connections are not opened automatically
  con <- curl(sprintf("telnet://127.0.0.1:%d", port), "r+b")
  on.exit(close(con), add = TRUE)
  expect_false(isOpen(con))
  open(con, "rb")
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
  con <- curl(sprintf("telnet://127.0.0.1:%d", port), "r+b")
  on.exit(close(con), add = TRUE)
  open(con, "rb", blocking = FALSE)
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

test_that("socket connection speaks IMAP over TLS", {
  skip_on_cran()
  skip_if_offline("imap.gmail.com")
  con <- curl("imaps://imap.gmail.com", "r+")
  on.exit(close(con), add = TRUE)
  open(con)
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
