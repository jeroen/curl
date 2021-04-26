context("echo")

test_that("roundtrip large data", {
  skip_if_not_installed('httpuv')
  bindata <- serialize(rnorm(1e5), NULL)
  input_url <- 'https://fakeserver.org:99/my/endpoint'
  handle <- curl::new_handle(url = input_url)
  handle_setheaders(handle, 'Foobar' = 'testtest')
  curl::handle_setform(handle, data =  curl::form_data(bindata, "application/octet-stream"))
  formdata <- curl::curl_echo(handle = handle)
  expect_identical(formdata$url, input_url)
  expect_identical(formdata$path, '/my/endpoint')
  expect_identical(formdata$method, 'POST')
  expect_identical(formdata$headers[['foobar']], 'testtest')
  expect_identical(formdata$headers[['host']], "fakeserver.org:99")
  out <- webutils::parse_http(formdata$body, formdata$content_type)
  expect_identical(out$data$value, bindata)
})
