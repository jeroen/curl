# curl

> A Modern and Flexible Web Client for R

[![CRAN_Status_Badge](https://www.r-pkg.org/badges/version/curl)](https://cran.r-project.org/package=curl)
[![CRAN RStudio mirror downloads](https://cranlogs.r-pkg.org/badges/curl)](https://cran.r-project.org/package=curl)

Bindings to [libcurl](https://curl.se/libcurl/) for performing fully
customizable HTTP/FTP/SCP requests where responses can be processed either in
memory, on disk, or streaming via the callback or connection interfaces. Some 
knowledge of 'libcurl' is recommended; for a more-user-friendly web client see
the 'httr2' package which builds on this package with http specific tools and logic

## Installation

The latest version of the package can be installed from r-universe:

```r
install.packages("curl", repos = "https://jeroen.r-universe.dev")
```

Other resources to get started:

 - [Intro vignette](https://cran.r-project.org/web/packages/curl/vignettes/intro.html): *curl - A Modern and Flexible Web Client for R*
 - [R-universe page](https://jeroen.r-universe.dev/curl) latest development information and resources 
 - [Libcurl handle options](https://curl.se/libcurl/c/curl_easy_setopt.html): overview of available options in libcurl

## Install from source on Linux / MacOS

Installation from source on Linux requires [`libcurl`](https://curl.se/libcurl/). On __Debian__ or __Ubuntu__ use [libcurl4-openssl-dev](https://packages.debian.org/testing/libcurl4-openssl-dev):

```
sudo apt-get install -y libcurl-dev
```

On __Fedora__, __CentOS or RHEL__ use [libcurl-devel](https://src.fedoraproject.org/rpms/curl):

```
sudo yum install libcurl-devel
````

On __MacOS__ libcurl is included with the system, so usually nothing extra is needed. However if you want to test using the very most recent version of libcurl you can install [curl from homebrew](https://github.com/Homebrew/homebrew-core/blob/master/Formula/c/curl.rb) and then recompile the R package:


```sh
brew install curl pkg-config
```

You need to set the `PKG_CONFIG_PATH` environment variable to help R find the non-default curl, when building from source. Run this in a __clean R session__ which does not have the curl package loaded already:

```r
Sys.setenv(PKG_CONFIG_PATH="/opt/homebrew/opt/curl/lib/pkgconfig:/usr/local/opt/curl/lib/pkgconfig")
install.packages("curl", type = "source")
```
