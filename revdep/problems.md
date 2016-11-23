# Setup

## Platform

|setting  |value                        |
|:--------|:----------------------------|
|version  |R version 3.3.2 (2016-10-31) |
|system   |x86_64, darwin13.4.0         |
|ui       |X11                          |
|language |(EN)                         |
|collate  |en_US.UTF-8                  |
|tz       |Europe/Amsterdam             |
|date     |2016-11-23                   |

## Packages

|package |*  |version |date       |source                     |
|:-------|:--|:-------|:----------|:--------------------------|
|curl    |   |2.3     |2016-11-23 |local (jeroenooms/curl@NA) |

# Check results

2 packages with problems

|package    |version | errors| warnings| notes|
|:----------|:-------|------:|--------:|-----:|
|data.table |1.9.6   |      1|        0|     1|
|smapr      |0.0.1   |      1|        0|     0|

## data.table (1.9.6)
Maintainer: Matt Dowle <mattjdowle@gmail.com>  
Bug reports: https://github.com/Rdatatable/data.table/issues

1 error  | 0 warnings | 1 note 

```
checking tests ... ERROR
Running the tests in ‘tests/tests.R’ failed.
Last 13 lines of output:
  > y = with(DT, eval(ll)) 
  First 6 of 644 :[1] 742 692 625 943 694 365
  forder decreasing argument test: seed = 1479917387  colorder = 2,4,1,3,5 
  Tests 1372.3+ not run. If required call library(GenomicRanges) first.
  Tests 1441-1444 not run. If required install the 'fr_FR.utf8' locale.
  
  Error in eval(expr, envir, enclos) : 
    14 errors out of 4390 (lastID=1557.4, endian=little, sizeof(long double)==16) in inst/tests/tests.Rraw on Wed Nov 23 17:09:55 2016. Search tests.Rraw for test numbers: 167, 167.2, 1253.152, 1253.156, 1253.232, 1253.234, 1253.24, 1253.242, 1253.264, 1253.272, 1253.424, 1253.428, 1253.44, 1253.444.
  Calls: test.data.table -> sys.source -> eval -> eval
  In addition: Warning message:
  In library(package, lib.loc = lib.loc, character.only = TRUE, logical.return = TRUE,  :
    there is no package called 'GenomicRanges'
  Execution halted

checking package dependencies ... NOTE
Package suggested but not available for checking: ‘GenomicRanges’
```

## smapr (0.0.1)
Maintainer: Maxwell Joseph <maxwell.b.joseph@colorado.edu>

1 error  | 0 warnings | 0 notes

```
checking package dependencies ... ERROR
Package required but not available: ‘rhdf5’

See section ‘The DESCRIPTION file’ in the ‘Writing R Extensions’
manual.
```

