test_that("Basic URL parser",{
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

test_that("Consistent URL encoding", {
  url1 <- "https://ja.wikipedia.org/wiki/\u5bff\u53f8"
  url2 <- "https://ja.wikipedia.org/wiki/%e5%af%bf%e5%8f%b8"
  out1 <- parse_url(url1)
  out2 <- parse_url(url2)
  out3 <- parse_url(url1, decode = FALSE)
  out4 <- parse_url(url2, decode = FALSE)
  expect_equal(out1$url, url1)
  expect_equal(out1$path, sub("^.*/wiki", "/wiki", url1))
  expect_equal(out2$url, url1)
  expect_equal(out2$path, sub("^.*/wiki", "/wiki", url1))
  expect_equal(tolower(out3$url), url2)
  expect_equal(tolower(out3$path), sub("^.*/wiki", "/wiki", url2))
  expect_equal(tolower(out4$url), url2)
  expect_equal(tolower(out4$path), sub("^.*/wiki", "/wiki", url2))
})
