# Note: we can only lookup symbols that are available in the installed version of libcurl
# Therefore you should only update the symbol table using the latest version of libcurl.
# On Mac: 'brew install curl' will install to /usr/local/opt/curl

# Function to read a symbol
library(inline)
getsymbol <- function(name){
  cat("Checking:", name, "\n")
  cfunction(
    cppargs="-I/usr/local/opt/curl/include",
    includes = '#include <curl/curl.h>',
    body = paste("return ScalarInteger((int)", name, ");")
  )()
}

# The symbols-in-versions file is included with libcurl
txt <- scan("tools/symbols-in-versions", character(), sep = "\n", skip = 13)
lines <- strsplit(txt, "[ ]+")
symbols <- as.data.frame(t(vapply(lines, `[`, character(4), 1:4)), stringsAsFactors = FALSE)
names(symbols) <- c("name", "introduced", "deprecated", "removed")

# Get current version
avail <- is.na(symbols$removed)

# Lookup all symbol values from curl.h (takes a while)
symbols$value <- NA_integer_;
symbols$value[avail] <- vapply(symbols$name[avail], getsymbol, integer(1))

# Compute type for options
type_name <- c("integer", "string", "function", "number")
type <- cut(symbols$value, c(-Inf, 0, 10000, 20000, 30000, 40000, Inf),
  labels = FALSE, right = FALSE)
type[is.na(type)] <- 1

symbols$type <- c("unknown", "integer", "string", "function", "number", "unknown")[type]

option <- grepl("CURLOPT", symbols$name)
symbols$type[!option] <- NA

# Save as lazy data
curl_symbols <- symbols[order(symbols$name), ]
devtools::use_data(curl_symbols, overwrite = TRUE)
