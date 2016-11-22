context("Nonblocking connection")

test_that("Non blocking connections ", {
  h <- new_handle()
  con <- curl(httpbin("drip?duration=3&numbytes=50&code=200"), handle = h)
  expect_equal(handle_data(h)$status_code, 0L)
  open(con, "rb", blocking = FALSE)
  expect_equal(handle_data(h)$status_code, 200L)
  n <- 0
  while(isIncomplete(con)){
    Sys.sleep(0.01)
    buf <- readBin(con, raw(), 1024)
    n <- n + length(buf)
  }
  expect_equal(n, 50L)
  rm(h)
  close(con)
  gc()
  expect_equal(total_handles(), 0L)
})

test_that("Non blocking readline", {
  con <- curl(httpbin("stream/71"))
  open(con, "rb", blocking = FALSE)
  n <- 0
  while(isIncomplete(con)){
    buf <- readLines(con)
    n <- n + length(buf)
  }
  expect_equal(n, 71L)
  close(con)
  gc()
  expect_equal(total_handles(), 0L)
})
