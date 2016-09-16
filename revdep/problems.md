# Setup

## Platform

|setting  |value                        |
|:--------|:----------------------------|
|version  |R version 3.3.1 (2016-06-21) |
|system   |x86_64, darwin13.4.0         |
|ui       |X11                          |
|language |(EN)                         |
|collate  |en_US.UTF-8                  |
|tz       |Europe/Amsterdam             |
|date     |2016-09-16                   |

## Packages

|package |*  |version |date       |source           |
|:-------|:--|:-------|:----------|:----------------|
|curl    |   |2.0     |2016-09-16 |local (NA/NA@NA) |

# Check results
2 packages with problems

|package    |version | errors| warnings| notes|
|:----------|:-------|------:|--------:|-----:|
|data.table |1.9.6   |      1|        0|     1|
|xml2       |1.0.0   |      1|        0|     0|

## data.table (1.9.6)
Maintainer: Matt Dowle <mattjdowle@gmail.com>  
Bug reports: https://github.com/Rdatatable/data.table/issues

1 error  | 0 warnings | 1 note 

```
checking tests ... ERROR
Running the tests in ‘tests/tests.R’ failed.
Last 13 lines of output:
  > y = with(DT, eval(ll)) 
  First 6 of 647 :[1]   1 109 137 368 350 883
  forder decreasing argument test: seed = 1474032946  colorder = 1,3,4,5,2 
  Tests 1372.3+ not run. If required call library(GenomicRanges) first.
  Tests 1441-1444 not run. If required install the 'fr_FR.utf8' locale.
  
  Error in eval(expr, envir, enclos) : 
    12 errors out of 4390 (lastID=1557.4, endian=little, sizeof(long double)==16) in inst/tests/tests.Rraw on Fri Sep 16 15:35:53 2016. Search tests.Rraw for test numbers: 1253.16, 1253.164, 1253.28, 1253.288, 1253.296, 1253.298, 1253.304, 1253.306, 1253.488, 1253.49, 1253.504, 1253.506.
  Calls: test.data.table -> sys.source -> eval -> eval
  In addition: Warning message:
  In library(package, lib.loc = lib.loc, character.only = TRUE, logical.return = TRUE,  :
    there is no package called 'GenomicRanges'
  Execution halted

checking package dependencies ... NOTE
Package suggested but not available for checking: ‘GenomicRanges’
```

## xml2 (1.0.0)
Maintainer: Hadley Wickham <hadley@rstudio.com>  
Bug reports: https://github.com/hadley/xml2/issues/

1 error  | 0 warnings | 0 notes

```
checking whether package ‘xml2’ can be installed ... ERROR
Installation failed.
See ‘/Users/jeroen/workspace/curl/revdep/checks/xml2.Rcheck/00install.out’ for details.
```

