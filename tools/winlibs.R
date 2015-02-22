# Build against static libraries from curl website.
if(!file.exists("../windows/libcurl-7.39.0/include/curl/curl.h")){
  setInternet2()
  download.file("https://github.com/rwinlib/libcurl/archive/v7.39.0.zip", "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
}
