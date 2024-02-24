context("SSL3 rpoblem")

test_that("problem due to SSL3 does not happen", {
  skip_on_cran()
  req <- curl::curl_fetch_memory('http://rest.kegg.jp/conv/ncbi-geneid/dme')
})
