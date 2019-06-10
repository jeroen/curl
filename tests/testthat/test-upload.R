context("Upload File")

test_that("File upload", {
  file <- system.file('DESCRIPTION', package = 'curl')
  res <- curl_upload(file, httpbin('/anything'), verbose = FALSE)
  data <- jsonlite::parse_json(rawToChar(res$content))$data
  input <- rawToChar(readBin(file, raw(), file.info(file)$size))
  expect_identical(data, input)
})
