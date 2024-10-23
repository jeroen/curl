test_that("Basic URL parser",{
  version <- as.numeric_version(curl::curl_version()$version)
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

test_that("Relative links need a baseurl",{
  base <- 'https://jerry:secret@google.com:888/foo/bar#nothing'
  out1 <- parse_url("/test1", base) #NB: curl does but ADA does not have nice error messages
  out2 <- parse_url("./test2", base)
  expect_equal(out1$url, "https://jerry:secret@google.com:888/test1")
  expect_equal(out2$url, "https://jerry:secret@google.com:888/foo/test2")
  expect_error(parse_url("/test1"))
  expect_error(parse_url("./test2"))
})
