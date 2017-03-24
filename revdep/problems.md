# Setup

## Platform

|setting  |value                        |
|:--------|:----------------------------|
|version  |R version 3.3.3 (2017-03-06) |
|system   |x86_64, darwin13.4.0         |
|ui       |X11                          |
|language |(EN)                         |
|collate  |en_US.UTF-8                  |
|tz       |Europe/Amsterdam             |
|date     |2017-03-24                   |

## Packages

|package |*  |version  |date       |source           |
|:-------|:--|:--------|:----------|:----------------|
|curl    |   |2.3.9000 |2017-03-24 |local (NA/NA@NA) |

# Check results
4 packages with problems

## biomartr (0.4.0)
Maintainer: Hajk-Georg Drost <hgd23@cam.ac.uk>  
Bug reports: https://github.com/HajkD/biomartr/issues

0 errors | 1 warning  | 0 notes

```
checking re-building of vignette outputs ... WARNING
Error in re-building vignettes:
  ...
Quitting from lines 29-31 (Database_Retrieval.Rmd) 
Error: processing vignette 'Database_Retrieval.Rmd' failed with diagnostics:
response reading failed
Execution halted

```

## data.table (1.10.4)
Maintainer: Matt Dowle <mattjdowle@gmail.com>  
Bug reports: https://github.com/Rdatatable/data.table/issues

1 error  | 0 warnings | 1 note 

```
checking tests ... ERROR
  Running ‘autoprint.R’
  Comparing ‘autoprint.Rout’ to ‘autoprint.Rout.save’ ... OK
  Running ‘knitr.R’
  Comparing ‘knitr.Rout’ to ‘knitr.Rout.save’ ... OK
  Running ‘main.R’ [118s/121s]
Running the tests in ‘tests/main.R’ failed.
Last 13 lines of output:
  Running test id 1696     Test 1751 ran without errors but failed check that x equals y:
  > x = capture.output(fwrite(DT, verbose = FALSE))[-1] 
  First 6 of 12 :[1] "1970-01-18T01:44:36.600000000Z" "1970-01-18T01:46:33.540000000Z"
  [3] "1970-01-18T01:46:33.540000000Z" "1970-01-01T00:00:00.061000001Z"
  [5] "1970-01-01T00:00:00.000000000Z" "1969-12-31T23:59:59.999999999Z"
  > y = tt 
  First 6 of 12 :[1] "2016-09-28T15:30:00.000000070Z" "2016-09-29T23:59:00.000000001Z"
  [3] "2016-09-29T23:59:00.000000999Z" "1970-01-01T00:01:01.000001000Z"
  [5] "1970-01-01T00:00:00.000000000Z" "1969-12-31T23:59:59.999999999Z"
  10 string mismatches
  
  Error in eval(expr, envir, enclos) : 
    161 errors out of 5901 (lastID=1751, endian==little, sizeof(long double)==16, sizeof(pointer)==8) in inst/tests/tests.Rraw on Fri Mar 24 12:07:15 2017. Search tests.Rraw for test numbers: 1253.248, 1253.25, 1253.252, 1253.254, 1253.256, 1253.258, 1253.26, 1253.262, 1253.264, 1253.266, 1253.268, 1253.27, 1253.272, 1253.274, 1253.276, 1253.278, 1253.28, 1253.282, 1253.284, 1253.286, 1253.288, 1253.29, 1253.292, 1253.294, 1253.312, 1253.314, 1253.316, 1253.318, 1253.32, 1253.322, 1253.324, 1253.326, 1253.328, 1253.33, 1253.332, 1253.334, 1253.336, 1253.338, 1253.34, 1253.342, 1253.36, 1253.362, 1253.364, 1253.366, 1253.368, 1253.37, 1253.372, 1253.374, 1253.392, 1253.394, 1253.396, 1253.398, 1253.4, 1253.402, 1253.404, 1253.406, 1253.408, 1253.41, 1253.412, 1253.414, 1253.416, 1253.418, 1253.42, 1253.422, 1253.424, 1253.426, 1253.428, 1253.43, 1253.432, 1253.434, 1253.436, 1253.438, 1253.44, 1253.442, 1253.444, 1253.446, 1253.448, 1253.45, 1253.452, 1253.454, 1253.456, 1253.458,
  Calls: test.data.table -> sys.source -> eval -> eval
  Execution halted

checking package dependencies ... NOTE
Package suggested but not available for checking: ‘GenomicRanges’
```

## geoknife (1.5.4)
Maintainer: Jordan Read <jread@usgs.gov>  
Bug reports: https://github.com/USGS-R/geoknife/issues

1 error  | 0 warnings | 0 notes

```
checking tests ... ERROR
  Running ‘testthat.R’
Running the tests in ‘tests/testthat.R’ failed.
Last 13 lines of output:
  
  The following object is masked from 'package:graphics':
  
      title
  
  The following object is masked from 'package:base':
  
      url
  
  Error in curl::curl_fetch_memory(url, handle = handle) : 
    Server returned nothing (no headers, no data)
  Calls: test_check ... request_fetch -> request_fetch.write_memory -> <Anonymous> -> .Call
  testthat results ================================================================
  OK: 32 SKIPPED: 19 FAILED: 0
  Execution halted
```

## textreadr (0.3.1)
Maintainer: Tyler Rinker <tyler.rinker@gmail.com>  
Bug reports: https://github.com/trinker/textreadr/issues?state=open

1 error  | 0 warnings | 0 notes

```
checking examples ... ERROR
Running examples in ‘textreadr-Ex.R’ failed
The error most likely occurred in:

> base::assign(".ptime", proc.time(), pos = "CheckExEnv")
> ### Name: read_dir_transcript
> ### Title: Read In Multiple Transcript Files From a Directory
> ### Aliases: read_dir_transcript
> 
> ### ** Examples
> 
> skips <- c(0, 1, 1, 0, 0, 1)
> path <- system.file("docs/transcripts", package = 'textreadr')
> textreadr::peek(read_dir_transcript(path, skip = skips), Inf)
R(44248,0x7fffbad4e3c0) malloc: *** error for object 0x10242e727: pointer being freed was not allocated
*** set a breakpoint in malloc_error_break to debug
```

