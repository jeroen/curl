# Build against static libraries from curl website.
if(!file.exists("../windows/curl-7.39.0-devel/include/curl/curl.h")){
  setInternet2()
  download.file("http://jeroenooms.github.io/curl/windows/curl-7.39.0-devel.zip", "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
}
