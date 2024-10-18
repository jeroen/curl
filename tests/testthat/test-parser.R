test_that("Basic URL parser",{
  version <- as.numeric_version(curl::curl_version()$version)
  skip_if(version < "7.62")
  url <- 'https://jerry:secret@google.com:888/foo/bar?test=123#bla'
  out <- parse_url(url)
  expect_equal(out$url, url)
  expect_equal(out$scheme, 'https')
  expect_equal(out$host, 'google.com')
  expect_equal(out$port, '888')
  expect_equal(out$path, '/foo/bar')
  expect_equal(out$query, "test=123")
  expect_equal(out$fragment, 'bla')
  expect_equal(out$user, 'jerry')
  expect_equal(out$password, 'secret')
})

