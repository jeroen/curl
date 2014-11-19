# Build against openssl libraries that were compiled with the Rtools gcc toolchain.
setInternet2()
download.file("http://jeroenooms.github.io/curl/windows/curl-7.39.0-devel.zip", "lib.zip", quiet = TRUE)
dir.create("../windows", showWarnings = FALSE)
unzip("lib.zip", exdir = "../windows")
