
context("interrupts")

test_that("interrupts are suspended in progress callbacks", {
  skip_on_cran()
  calls <- 0L
  progress <- function(down, up) {
    calls <<- calls + 1L
    if (calls >= 2 && calls <= 5) {
      curl:::interrupt_me()
    }
    TRUE
  }

  h <- new_handle(progressfunction = progress, noprogress = FALSE)
  exitcond <- NULL
  tryCatch(
    ret <- curl_fetch_memory("https://cloud.r-project.org/web/packages/curl/curl.pdf", handle = h),
    interrupt = function(cond) exitcond <<- cond,
    error = function(cond) exitcond <<- cond,
    condition = function(cond) exitcond <<- cond
  )
  expect_false(is.null(exitcond))
  expect_s3_class(exitcond, "interrupt")
})
