context("Multi handle")

test_that("Max connections works", {
  skip_if_not(curl_version()$version >= as.numeric_version("7.30"),
    "libcurl does not support host_connections")
  multi_set(host_con = 1, multiplex = FALSE)
  for(i in 1:3){
    multi_add(new_handle(url = "https://eu.httpbin.org/delay/2"))
  }
  out <- multi_run(timeout = 3)
  expect_equal(out, list(success = 1, error = 0, pending = 2))
  out <- multi_run(timeout = 2)
  expect_equal(out, list(success = 1, error = 0, pending = 1))
  out <- multi_run()
  expect_equal(out, list(success = 1, error = 0, pending = 0))
})

test_that("Max connections reset", {
  for(i in 1:3){
    multi_add(new_handle(url = "https://eu.httpbin.org/delay/2"))
  }
  multi_set(host_con = 6)
  out <- multi_run(timeout = 4)
  expect_equal(out, list(success = 3, error = 0, pending = 0))
})

test_that("Timeout works", {
  h1 <- new_handle(url = "https://eu.httpbin.org/delay/3")
  h2 <- new_handle(url = "https://eu.httpbin.org/post", postfields = "bla bla")
  h3 <- new_handle(url = "https://urldoesnotexist.xyz", connecttimeout = 1)
  h4 <- new_handle(url = "http://localhost:14", connecttimeout = 1)
  multi_add(h1)
  multi_add(h2)
  multi_add(h3)
  multi_add(h4)
  rm(h1, h2, h3, h4)
  gc()
  out <- multi_run(timeout = 2)
  expect_equal(out, list(success = 1, error = 2, pending = 1))
  out <- multi_run(timeout = 0)
  expect_equal(out, list(success = 0, error = 0, pending = 1))
  out <- multi_run()
  expect_equal(out, list(success = 1, error = 0, pending = 0))
})

test_that("Callbacks work", {
  total = 0;
  h1 <- new_handle(url = "https://eu.httpbin.org/get")
  multi_add(h1, done = function(...){
    total <<- total + 1
    multi_add(h1, done = function(...){
      total <<- total + 1
    })
  })
  gc() # test that callback functions are protected
  out <- multi_run()
  expect_equal(out, list(success=2, error=0, pending=0))
  expect_equal(total, 2)
})

test_that("Multi cancel works", {
  h1 <- new_handle(url = "https://eu.httpbin.org/get")
  multi_add(h1)
  expect_error(multi_add(h1), "locked")
  expect_equal(multi_run(timeout = 0), list(success = 0, error = 0, pending = 1))
  expect_is(multi_cancel(h1), "curl_handle")
  expect_is(multi_add(h1), "curl_handle")
  expect_equal(multi_run(), list(success = 1, error = 0, pending = 0))
})
