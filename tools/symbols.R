# Function to read a symbol
library(inline)
getsymbol <- function(name){
  cfunction(includes = '#include "/Users/jeroen/Downloads/curl-7.41.0/include/curl/curl.h"', body = paste("return ScalarInteger((int)", name, ");"))()
}

# The symbols-in-versions file is included with libcurl
txt <- scan("symbols-in-versions", character(), sep = "\n", skip = 13)
lines <- strsplit(txt, "[ ]+")
symbols <- as.data.frame(t(vapply(lines, `[`, character(4), 1:4)), stringsAsFactors = FALSE)
names(symbols) <- c("name", "introduced", "deprecated", "removed")

# Get current version
library(curl)
version <- curl_version()$version
avail <- (vapply(symbols$introduced, compareVersion, double(1), a = version, USE.NAMES=FALSE) > -1) & is.na(symbols$removed)

# Lookup all symbol values from curl.h (takes a while)
symbols$value <- NA;
symbols$value[avail] <- vapply(symbols$name[avail], getsymbol, integer(1))

# Save as lazy data
curl_symbols <- symbols
save(curl_symbols, file = "../R/sysdata.rda")
