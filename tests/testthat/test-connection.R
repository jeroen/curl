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
