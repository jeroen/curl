test_that("test error classes", {
  # Throw an actual error
  expect_error(curl::curl_fetch_memory("https://urldoesnotexist.xyz"), class = 'curl_error_couldnt_resolve_host')

  # This handle raises an error
  error1 <- NULL
  error2 <- NULL
  pool <- new_pool()
  curl::curl_fetch_multi("https://urldoesnotexist.xyz", fail = function(err){
    error1 <<- err
  }, pool = pool)
  curl::curl_fetch_multi("https://urldoesnotexist.xyz", fail = function(){
    error2 <<- TRUE
  }, pool = pool)
  curl::curl_fetch_multi("https://urldoesnotexist.xyz", pool = pool)
  out <- multi_run(pool = pool)
  expect_equal(out$error, 3)
  expect_s3_class(error1, 'curl_error_couldnt_resolve_host')
  expect_true(error2)
})
