context("Garbage collection")

test_that("callbacks are protected", {
  progress <- function(down, up) { TRUE }
  h <- new_handle(progressfunction = progress, noprogress = FALSE)
  rm(progress)
  gc()
  res <- curl_fetch_memory("https://cloud.r-project.org/web/packages/curl/curl.pdf", handle = h)
  expect_equal(res$status, 200)
  expect_equal(total_handles(), 1L)
  rm(h)
  gc()
  expect_equal(total_handles(), 0L)
})
