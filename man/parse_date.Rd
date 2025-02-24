% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/utilities.R
\name{parse_date}
\alias{parse_date}
\title{Parse date/time}
\usage{
parse_date(datestring)
}
\arguments{
\item{datestring}{a string consisting of a timestamp}
}
\description{
Can be used to parse dates appearing in http response headers such
as \code{Expires} or \code{Last-Modified}. Automatically recognizes
most common formats. If the format is known, \code{\link[=strptime]{strptime()}}
might be easier.
}
\examples{
# Parse dates in many formats
parse_date("Sunday, 06-Nov-94 08:49:37 GMT")
parse_date("06 Nov 1994 08:49:37")
parse_date("20040911 +0200")
}
