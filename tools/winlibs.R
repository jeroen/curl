# Build against static libraries from rwinlib
if(!file.exists("../windows/libcurl-7.51.0/include/curl/curl.h")){
  if(getRversion() < "3.3.0") setInternet2()
  download.file("https://github.com/rwinlib/libcurl/archive/v7.51.0.zip", "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
}
