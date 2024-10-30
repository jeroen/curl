test_that("Seeking works after redirect",{
  skip_on_cran()
  str <- paste(letters, collapse = '')
  tmp <- tempfile()
  writeBin(str, tmp)
  url <- httpbin('/redirect-to?url=/put&status_code=307')
  req <- curl::curl_upload(tmp, url, verbose = FALSE)
  headers <- curl::parse_headers(req$headers, multiple = TRUE)
  expect_length(headers, 2)
  expect_equal(req$status_code, 200)
  content <- jsonlite::fromJSON(rawToChar(req$content))$data
  expect_equal(content, str)
})
