% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/options.R, R/utilities.R
\name{curl_options}
\alias{curl_options}
\alias{curl_options_table}
\alias{curl_symbols}
\alias{curl_version}
\title{List curl version and options.}
\usage{
curl_options(filter = "")

curl_options_table(filter = "")

curl_symbols(filter = "")

curl_version()
}
\arguments{
\item{filter}{string: only return options with string in name}
}
\description{
\code{curl_version()} shows the versions of libcurl, libssl and zlib and
supported protocols. \code{curl_options()} lists all options available in
the current version of libcurl.  The dataset \code{curl_symbols} lists all
symbols (including options) provides more information about the symbols,
including when support was added/removed from libcurl.
}
\examples{
# Available options
curl_options()

# List proxy options
curl_options("proxy")

# Symbol table
curl_symbols("proxy")
# Curl/ssl version info
curl_version()
}
