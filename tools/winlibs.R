if(!file.exists("../windows/libcurl/include/curl/curl.h")){
  unlink("../windows", recursive = TRUE)
  url <- if(grepl("aarch", R.version$platform)){
    "https://github.com/r-windows/bundler/releases/download/curl-8.1.2-9000/curl-8.1.2-9000-clang-aarch64.tar.xz"
  } else if(getRversion() >= "4.2") {
    "https://github.com/r-windows/bundler/releases/download/curl-8.1.2-9000/curl-8.1.2-9000-ucrt-x86_64.tar.xz"
  } else {
    "https://github.com/rwinlib/libcurl/archive/v7.84.0.tar.gz"
  }
  download.file(url, basename(url), quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  untar(basename(url), exdir = "../windows", tar = 'internal')
  unlink(basename(url))
  setwd("../windows")
  file.rename(list.files(), 'libcurl')
}
