# curl

##### *A Modern and Flexible Web Client for R*

[![Build Status](https://travis-ci.org/jeroen/curl.svg?branch=master)](https://travis-ci.org/jeroen/curl)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/jeroen/curl?branch=master&svg=true)](https://ci.appveyor.com/project/jeroen/curl)
[![Coverage Status](https://codecov.io/github/jeroen/curl/coverage.svg?branch=master)](https://codecov.io/github/jeroen/curl?branch=master)
[![CRAN_Status_Badge](http://www.r-pkg.org/badges/version/curl)](http://cran.r-project.org/package=curl)
[![CRAN RStudio mirror downloads](http://cranlogs.r-pkg.org/badges/curl)](http://cran.r-project.org/web/packages/curl/index.html)
[![Research software impact](http://depsy.org/api/package/cran/curl/badge.svg)](http://depsy.org/package/r/curl)
[![Github Stars](https://img.shields.io/github/stars/jeroen/curl.svg?style=social&label=Github)](https://github.com/jeroen/curl)

> The curl() and curl_download() functions provide highly
  configurable drop-in replacements for base url() and download.file() with
  better performance, support for encryption (https, ftps), gzip compression,
  authentication, and other libcurl goodies. The core of the package implements a
  framework for performing fully customized requests where data can be processed
  either in memory, on disk, or streaming via the callback or connection
  interfaces. Some knowledge of libcurl is recommended; for a more-user-friendly
  web client see the 'httr' package which builds on this package with http
  specific tools and logic.

## Devel version

Note that you cannot use `devtools::install_github()` because it uses `curl` ;)

```r
install.packages("https://github.com/jeroen/curl/archive/master.tar.gz", repos = NULL)
```

## Documentation

About the R package:

 - Vignette: [curl: A Modern and Flexible Web Client for R](https://cran.r-project.org/web/packages/curl/vignettes/intro.html)

Other resources:

 - [libcurl handle options overview](https://curl.haxx.se/libcurl/c/curl_easy_setopt.html) (use with `handle_setopt` in R)

## Hello World

There are three download interfaces (memory, disk and streaming). Always start by setting up a request handle:

```r
library(curl)
h <- new_handle(copypostfields = "moo=moomooo")
handle_setheaders(h,
  "Content-Type" = "text/moo",
  "Cache-Control" = "no-cache",
  "User-Agent" = "A cow"
)
```

Perform request and download response in memory:

```r
# Perform the request
req <- curl_fetch_memory("http://httpbin.org/post", handle = h)

# Show some outputs
parse_headers(req$headers)
cat(rawToChar(req$content))
str(req)
```

Or alternatively, write response to disk:

```r
tmp <- tempfile()
curl_download("https://httpbin.org/post", tmp, handle = h)
readLines(tmp)
```

Or stream response via Connection interface:

```r
con <- curl("https://httpbin.org/post", handle = h)
open(con)

# Get 3 lines
readLines(con, n = 3)

# Get remaining lines and close connection
readLines(con)
close(con)
```

## Installation

Binary packages for __OS-X__ or __Windows__ can be installed directly from CRAN:

```r
install.packages("curl")
```

Installation from source on Linux requires [`libcurl`](https://curl.haxx.se/libcurl/). On __Debian__ or __Ubuntu__ use [libcurl4-openssl-dev](https://packages.debian.org/testing/libcurl4-openssl-dev):

```
sudo apt-get install -y libcurl-dev
```

On __Fedora__, __CentOS or RHEL__ use [libcurl-devel](https://apps.fedoraproject.org/packages/libcurl-devel):

```
sudo yum install libcurl-devel
````

On __OS-X__ libcurl is included with the system so nothing extra is needed. However if you want to build against the most recent version of libcurl, install and force-link [curl from homebrew](https://github.com/Homebrew/homebrew-core/blob/master/Formula/curl.rb):

```
brew install curl
brew link --force curl
```

Note that on OS-X you **must** recompile the R package from source after force-linking curl, otherwise you get a version conflict with the system version of libcurl.

## Development version

Because `devtools` and `httr` depend on `curl`, installing with `install_github` does not work well. The easiest way to install the development version of `curl` is a clean R session:

```r
install.packages("https://github.com/jeroen/curl/archive/master.tar.gz", repos = NULL)
```

Of course windows users need [Rtools](https://cran.r-project.org/bin/windows/Rtools/) to compile from source.

## Enable HTTP/2

To use HTTP/2 the `libcurl` library has to be built `--with-nghttp2` which is usually not the default. Use `curl_version()` in R to check if it is enabled. To enable it on OSX use:

```
brew install curl --with-nghttp2
brew link curl --force
```

And then reinstall the `curl` package from source:

```r
install.packages("https://github.com/jeroen/curl/archive/master.tar.gz", repos = NULL)
```

Check again `curl_version()` to see if HTTP/2 is enabled now.

