# Build against static libraries from curl website.
if(!file.exists("../windows/libcurl-7.47.1/include/curl/curl.h")){
  if(getRversion() < "3.3.0") setInternet2()
  download.file("https://github.com/rwinlib/libcurl/archive/v7.47.1.zip", "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
}
