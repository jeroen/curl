context("Nonblocking connection")

test_that("Non blocking connections ", {
  h <- new_handle()
  con <- curl(httpbin("drip?duration=3&numbytes=50&code=200"), handle = h)
  expect_equal(handle_data(h)$status_code, 0L)
  open(con, "rb", blocking = FALSE)
  expect_equal(handle_data(h)$status_code, 200L)
  n <- 0
  while(identical(summary(con)[["can read"]], "yes")){
    Sys.sleep(0.1)
    buf <- readBin(con, raw(), 1024)
    n <- n + length(buf)
  }
  expect_equal(n, 50L)
  expect_equal(summary(con)[["can read"]], "no")
  expect_error(readBin(con, raw(), 1024), "read")
  rm(h)
  close(con)
  gc()
  expect_equal(total_handles(), 0L)
})

test_that("Non blocking readline", {
  con <- curl(httpbin("stream/71"))
  open(con, "rb", blocking = FALSE)
  n <- 0
  while(identical(summary(con)[["can read"]], "yes")){
    Sys.sleep(0.1)
    buf <- readLines(con)
    n <- n + length(buf)
  }
  expect_equal(n, 71L)
  expect_equal(summary(con)[["can read"]], "no")
  expect_error(readLines(con), "read")
  close(con)
  gc()
  expect_equal(total_handles(), 0L)
})
