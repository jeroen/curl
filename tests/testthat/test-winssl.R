context("SecureChannel")

test_that("Some legacy servers", {
  skip_on_cran()
  req <- curl::curl_fetch_memory('https://crandb.r-pkg.org/dplyr/all')
})
