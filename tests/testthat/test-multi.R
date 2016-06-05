context("Multi handle")

test_that("Max connections works", {
  skipifnot(curl_version()$version >= as.numeric_version("7.30"))
  for(i in 1:3){
    multi_add(new_handle(url = "https://httpbin.org/delay/3"))
  }
  out <- multi_run(host_connections = 1, timeout = 5)
  expect_equal(out, list(success = 1, error = 0, pending = 2))
  out <- multi_run(3, host_connections = 1)
  expect_equal(out, list(success = 1, error = 0, pending = 1))
  out <- multi_run(host_connections = 1)
  expect_equal(out, list(success = 1, error = 0, pending = 0))
})

test_that("Max connections reset", {
  for(i in 1:3){
    multi_add(new_handle(url = "https://httpbin.org/delay/3"))
  }
  out <- multi_run(host_connections = 6, timeout = 5)
  expect_equal(out, list(success = 3, error = 0, pending = 0))
})

test_that("Timeout works", {
  h1 <- new_handle(url = "https://httpbin.org/delay/5")
  h2 <- new_handle(url = "https://httpbin.org/post", postfields = "bla bla")
  h3 <- new_handle(url = "https://urldoesnotexist.xyz")
  multi_add(h1)
  multi_add(h2)
  multi_add(h3)
  out <- multi_run(timeout = 3)
  expect_equal(out, list(success=1, error=1, pending=1))
  out <- multi_run(0)
  expect_equal(out, list(success=0, error=0, pending=1))
  out <- multi_run()
  expect_equal(out, list(success=1, error=0, pending=0))
})

