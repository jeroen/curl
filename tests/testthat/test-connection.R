h <- new_handle()

test_that("Compression and destroying connection", {
  # Compression is broken in 8.7.1, see https://github.com/curl/curl/issues/13493
  skip_if(curl::curl_version()$version == '8.7.1')
  con <- curl(httpbin("deflate"), handle = h)
  expect_equal(jsonlite::fromJSON(readLines(con))$deflate, TRUE)
  expect_false(isOpen(con))
  close(con) #destroy

  expect_equal(jsonlite::fromJSON(rawToChar(curl_fetch_memory(httpbin("deflate"), handle = h)$content))$deflate, TRUE)

  con <- curl(httpbin("gzip"), handle = h)
  expect_equal(jsonlite::fromJSON(readLines(con))$gzipped, TRUE)
  expect_false(isOpen(con))
  close(con) #destroy

  expect_equal(jsonlite::fromJSON(rawToChar(curl_fetch_memory(httpbin("gzip"), handle = h)$content))$gzipped, TRUE)
})

test_that("Connection interface", {
  # note: jsonlite automatically destroys auto-opened connection
  con <- curl(httpbin("get?test=blabla"), handle = h)
  expect_equal(jsonlite::fromJSON(con)$args$test, "blabla")

  # test error
  con <- curl(httpbin("status/418"))
  expect_error(expect_warning(readLines(con), '418'), 'cannot open')
  close(con) #destroy

  # test not error
  con <- curl(httpbin("status/418"), handle = h)
  open(con, "rf")
  expect_is(readLines(con), "character")
  expect_equal(handle_data(h)$status_code, 418L)
  close(con) #destroy
})

test_that("Can retrieve fds from a connection", {
  con <- curl(httpbin("get?test=blabla"), handle = h)
  fds <- multi_fdset(con)
  expect_type(fds, "list")
  close(con)
  not <- "not a connection"
  class(not) <- "curl"
  expect_error(multi_fdset(not))
})

test_that("Blocking open returns once the headers are received", {
  skip_on_os("windows") # mcparallel() needs fork

  port <- find_port()
  listener <- serverSocket(port)
  on.exit(close(listener))
  server <- parallel::mcparallel({
    con <- socketAccept(listener, blocking = TRUE, open = "r+b")
    repeat {
      line <- readLines(con, n = 1)
      if (!length(line) || !nzchar(line)) break
    }
    writeLines(c("HTTP/1.1 200 OK", "Content-Length: 5", ""), con, sep = "\r\n")
    flush(con)
    Sys.sleep(2)
    writeBin(charToRaw("hello"), con)
    close(con)
  })
  on.exit(parallel::mccollect(server), add = TRUE)

  h <- new_handle()
  con <- curl(paste0("http://127.0.0.1:", port), handle = h)
  elapsed <- system.time(open(con, "rb"))[["elapsed"]]
  expect_lt(elapsed, 1)
  expect_equal(handle_data(h)$status_code, 200L)
  expect_equal(rawToChar(readBin(con, raw(), 10)), "hello")
  close(con)
})
