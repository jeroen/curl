context("Multi handle")

test_that("Timeout works", {
  skip_on_os("solaris")
  h1 <- new_handle(url = httpbin("delay/3"))
  h2 <- new_handle(url = httpbin("post"), postfields = "bla bla")
  h3 <- new_handle(url = "https://urldoesnotexist.xyz", connecttimeout = 1)
  h4 <- new_handle(url = "http://localhost:14", connecttimeout = 1)
  m <- new_pool()
  multi_add(h1, pool = m)
  multi_add(h2, pool = m)
  multi_add(h3, pool = m)
  multi_add(h4, pool = m)
  rm(h1, h2, h3, h4)
  gc()
  out <- multi_run(timeout = 2, pool = m)
  expect_equal(out, list(success = 1, error = 2, pending = 1))
  out <- multi_run(timeout = 0, pool = m)
  expect_equal(out, list(success = 0, error = 0, pending = 1))
  out <- multi_run(pool = m)
  expect_equal(out, list(success = 1, error = 0, pending = 0))
})

test_that("Callbacks work", {
  total = 0;
  h1 <- new_handle(url = httpbin("get"))
  multi_add(h1, done = function(...){
    total <<- total + 1
    multi_add(h1, done = function(...){
      total <<- total + 1
    })
  })
  gc() # test that callback functions are protected
  out <- multi_run()
  expect_equal(out$pending, 0)
  expect_equal(out$success + out$error, 2)
  expect_equal(total, 2)
})

test_that("Multi cancel works", {
  expect_length(multi_list(), 0)
  h1 <- new_handle(url = httpbin("get"))
  multi_add(h1)
  expect_length(multi_list(), 1)
  expect_error(multi_add(h1), "locked")
  expect_equal(multi_run(timeout = 0), list(success = 0, error = 0, pending = 1))
  expect_length(multi_list(), 1)
  expect_is(multi_cancel(h1), "curl_handle")
  expect_length(multi_list(), 0)
  expect_is(multi_add(h1), "curl_handle")
  expect_length(multi_list(), 1)
  expect_equal(multi_run(), list(success = 1, error = 0, pending = 0))
  expect_length(multi_list(), 0)
})

test_that("Errors in Callbacks", {
  pool <- new_pool()
  cb <- function(req){
    stop("testerror in callback!")
  }
  curl_fetch_multi(httpbin("get"), pool = pool, done = cb)
  curl_fetch_multi(httpbin("status/404"), pool = pool, done = cb)
  curl_fetch_multi("https://urldoesnotexist.xyz", pool = pool, fail = cb)
  gc()
  expect_equal(total_handles(), 3)
  expect_error(multi_run(pool = pool), "testerror")
  gc()
  expect_equal(total_handles(), 2)
  expect_error(multi_run(pool = pool), "testerror")
  gc()
  expect_equal(total_handles(), 1)
  expect_error(multi_run(pool = pool), "testerror")
  gc()
  expect_equal(total_handles(), 0)
  expect_equal(multi_run(pool = pool), list(success = 0, error = 0, pending = 0))
})

test_that("Data callback", {
  con <- rawConnection(raw(0), "r+")
  on.exit(close(con))
  hx <- new_handle()
  handle_setopt(hx, COPYPOSTFIELDS = jsonlite::toJSON(mtcars));
  handle_setheaders(hx, "Content-Type" = "application/json")
  status <- NULL
  curl_fetch_multi(httpbin("post"), done = function(res){
    status <<- res$status_code
  }, fail = stop, data = con, handle = hx)

  rawheaders <- NULL
  buffer <- raw()
  hx <- new_handle(accept_encoding = 'identity')
  curl_fetch_multi(httpbin("get"), handle = hx, done = function(res){
    rawheaders <<- res$headers
    #this somehow breaks the gc
    #expect_equal(res$status_code, 200)
  }, fail = stop, data = function(x, finalize = FALSE){
    buffer <<- c(buffer, x)
    # also breaks gc. Looks like circular protect caused by testthat?
    # expect_is(x, "raw")
  })

  # test that callback functions are protected
  gc()

  # perform requests
  out <- multi_run()
  expect_equal(out$success, 2)
  expect_equal(status, 200)

  # output from callback functions
  content_len <- curl::parse_headers_list(rawheaders)[['content-length']]
  expect_length(buffer, as.numeric(content_len))

  # get data from buffer
  content <- rawConnectionValue(con)
  output <- jsonlite::fromJSON(rawToChar(content))
  expect_is(output$json, "data.frame")
  expect_equal(sort(names(output$json)), sort(names(mtcars)))
})

test_that("callback protection", {
  done <- function(res){
    expect_is(res$status_code, "integer")
  }
  fail <- function(...){
    print("error")
  }
  data <- function(x, final){
    expect_is(x, "raw")
  }
  pool <- new_pool()
  handle <- new_handle(url = httpbin("get"))
  multi_add(handle, done = done, fail = fail, data = data, pool = pool)
  rm(handle, done, fail, data)
  gc(); gc();
  out <- multi_run(pool = pool)
  expect_equal(out$success, 1)
})

test_that("total_con and multi_fdset", {
  skip_on_os("solaris")
  skip_if_not(strsplit(curl_version()$version, "-")[[1]][1] >= as.numeric_version("7.30"),
              "libcurl does not support host_connections")
  total_con <- 5
  pool <- new_pool(total_con = total_con, multiplex = FALSE)
  for (i in c(4, 3, 2, 1, 0, 1, 2, 3, 4)) {
    h1 <- new_handle(url = httpbin(paste0("delay/", i)))
    multi_add(h1, done = force, fail = cat, pool = pool)
  }
  while(length(multi_list(pool = pool))){
    res <- multi_run(pool = pool, poll = 1)
    fdset <- multi_fdset(pool = pool)
    expect_length(c(fdset$reads, fdset$writes), min(total_con, res$pending))
  }
})

test_that("host_con and multi_fdset", {
  skip_on_os("solaris")
  skip_if_not(strsplit(curl_version()$version, "-")[[1]][1] >= as.numeric_version("7.30"),
              "libcurl does not support host_connections")
  host_con <- 5
  pool <- new_pool(host_con = host_con, multiplex = FALSE)
  for (i in c(4, 3, 2, 1, 0, 1, 2, 3, 4)) {
    h1 <- new_handle(url = httpbin(paste0("delay/", i)))
    multi_add(h1, done = force, fail = cat, pool = pool)
  }
  while(length(multi_list(pool = pool))){
    res <- multi_run(pool = pool, poll = 1)
    fdset <- multi_fdset(pool = pool)
    expect_length(c(fdset$reads, fdset$writes), min(host_con, res$pending))
  }
})

test_that("GC works", {
  gc()
  expect_equal(total_handles(), 0L)
})
