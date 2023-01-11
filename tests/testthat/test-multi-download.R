context("Multi download")

test_that("Stress test multi_download", {
  skip_on_cran()
  mirror <- 'https://cloud.r-project.org'
  pkgs <- row.names(available.packages(repos = mirror))[1:3000]
  urls <- sprintf('%s/web/packages/%s/DESCRIPTION', mirror, pkgs)
  outdir <- file.path(tempdir(), 'descriptions')
  files <- sprintf('%s/%s.txt', outdir, pkgs)
  dir.create(outdir, showWarnings = FALSE)
  results <- curl::multi_download(urls, files, timeout = 60, progress = FALSE)
  expect_length(results$success, length(pkgs))
  expect_true(all(results$success))
  expect_true(all(results$status_code == 200))
  expect_true(all(file.exists(results$destfile)))
  unlink(results$destfile)
})
