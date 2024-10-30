test_that("Support preconfigured handles", {
  expect_error(multi_download(new_handle()), 'no URL')
  expect_error(multi_download(new_handle(url = 'https://www.google.com')), 'filename')
  h1 <- new_handle(url = httpbin("html"))
  h2 <- new_handle(url = httpbin("robots.txt"))
  df <- multi_download(c(h1, h2), progress = FALSE)
  expect_equal(df$destfile, normalizePath(c('html', 'robots.txt')))
  expect_equal(df$status_code, c(200, 200))
  unlink(c('html', 'robots.txt'))
})


test_that("Stress test multi_download", {
  skip_on_cran()
  mirror <- 'https://cloud.r-project.org'
  pkgs <- row.names(available.packages(repos = mirror))[1:1000]
  urls <- sprintf('%s/web/packages/%s/DESCRIPTION', mirror, pkgs)
  outdir <- file.path(tempdir(), 'descriptions')
  files <- sprintf('%s/%s.txt', outdir, pkgs)
  dir.create(outdir, showWarnings = FALSE)
  results <- curl::multi_download(urls, files, multi_timeout = 60, progress = FALSE)
  expect_length(results$success, length(pkgs))
  expect_true(all(results$success))
  expect_true(all(results$status_code == 200))
  expect_true(all(file.exists(results$destfile)))
  unlink(results$destfile)
})
