library(testthat)
library(curl)

# Comply with CRAN policy: skip tests if no internet
if(curl::has_internet()){
  test_check("curl")
}
