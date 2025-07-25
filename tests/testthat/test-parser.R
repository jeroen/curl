test_that("Basic URL parser",{
  url <- 'https://jerry:secret@google.com:888/foo/bar?test=123#bla'
  out <- curl_parse_url(url)
  expect_equal(out$url, url)
  expect_equal(out$scheme, 'https')
  expect_equal(out$host, 'google.com')
  expect_equal(out$port, '888')
  expect_equal(out$path, '/foo/bar')
  expect_equal(out$params, c(test="123"))
  expect_equal(out$fragment, 'bla')
  expect_equal(out$user, 'jerry')
  expect_equal(out$password, 'secret')
  expect_equal(curl_modify_url(out), url)

  expect_equal(curl_modify_url(out), url)
  out$url <- NULL
  expect_equal(curl_modify_url(out), url)
})

test_that("Relative links need a baseurl",{
  base <- 'https://jerry:secret@google.com:888/foo/bar?x=1#nothing'
  out1 <- curl_parse_url("/test1", base)
  out2 <- curl_parse_url("./test2", base)
  out3 <- curl_parse_url("#bla", base)
  expect_equal(out1$url, "https://jerry:secret@google.com:888/test1")
  expect_equal(out2$url, "https://jerry:secret@google.com:888/foo/test2")
  expect_equal(out3$url, "https://jerry:secret@google.com:888/foo/bar?x=1#bla")
  expect_error(curl_parse_url("/test1"))
  expect_error(curl_parse_url("./test2"))
})

test_that("Consistent URL encoding", {
  url1 <- "https://ja.wikipedia.org/wiki/\u5bff\u53f8"
  url2 <- "https://ja.wikipedia.org/wiki/%E5%AF%BF%E5%8F%B8"
  out1 <- curl_parse_url(url1)
  out2 <- curl_parse_url(url2)
  out3 <- curl_parse_url(url1, decode = FALSE)
  out4 <- curl_parse_url(url2, decode = FALSE)
  expect_equal(out1$url, url2)
  expect_equal(out1$path, sub("^.*/wiki", "/wiki", url1))
  expect_equal(out2$url, url2)
  expect_equal(out2$path, sub("^.*/wiki", "/wiki", url1))
  expect_equal(out3$url, url2)
  expect_equal(out3$path, sub("^.*/wiki", "/wiki", url2))
  expect_equal(out4$url, url2)
  expect_equal(out4$path, sub("^.*/wiki", "/wiki", url2))

  expect_equal(curl_modify_url(out1), url2)
  expect_equal(curl_modify_url(out2), url2)
  out1$url <- NULL
  out2$url <- NULL
  expect_equal(curl_modify_url(out1), url2)
  expect_equal(curl_modify_url(out2), url2)
})

test_that("IPv6 address is understood", {
  out <- curl_parse_url('http://[2001:db8::1]:8080')
  expect_equal(out$host, '[2001:db8::1]')
  expect_equal(out$port, '8080')
})

test_that("Decoding parameters", {
  url <- 'https://www.test.com/bla?tv=tom%26jerry&math=1%2B1+%3D+2&empty='
  out <- curl_parse_url(url)
  expect_equal(out$params, c(tv = "tom&jerry", math = "1+1 = 2", empty = ""))
  expect_equal(curl_modify_url(out), url)
  out$url <- NULL
  expect_equal(curl_modify_url(out), url)
  expect_equal(curl_modify_url(out, query = '', path = ''), 'https://www.test.com/')
})

test_that("Using a default scheme", {
  expect_error(curl_parse_url('yolo'), 'parse')
  out <- curl_parse_url('yolo', default_scheme = TRUE)
  expect_equal(out$url, 'https://yolo/')
})
