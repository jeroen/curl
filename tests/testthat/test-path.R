context("Paths")

tricky_filename <- "\u00C0\u00CB\u00D0"

# https://github.com/jeroen/curl/issues/182
test_that("curl_download() and curl_fetch_disk() can write to a non-ASCII path", {
  ## chosen to be non-ASCII but
  ## [1] representable in Windows-1252 and
  ## [2] not any of the few differences between Windows-1252 and ISO-8859-1
  ## a-grave + e-diaeresis  + Eth

  tmpdir <- tempfile()
  dir.create(tmpdir)
  on.exit(unlink(tmpdir, recursive = TRUE))

  res <- curl_fetch_disk(httpbin("stream/10"), file.path(tmpdir, tricky_filename))
  expect_true(file.exists(res$content))
  unlink(list.files(tmpdir, full.names = TRUE))

  res <- curl_download(httpbin("stream/10"), file.path(tmpdir, tricky_filename))
  expect_true(file.exists(res))
})

test_that("curl_download() does not store failed downloads", {
  dest1 <- tempfile(pattern = tricky_filename)
  curl_download(httpbin("get"), destfile = dest1)
  expect_true(file.exists(dest1))
  unlink(dest1)

  dest2 <- tempfile()
  expect_error(curl_download(httpbin("status/404"), destfile = dest2))
  expect_true(!file.exists(dest2))
})
