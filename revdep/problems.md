# Setup

## Platform

|setting  |value                        |
|:--------|:----------------------------|
|version  |R version 3.3.1 (2016-06-21) |
|system   |x86_64, darwin13.4.0         |
|ui       |RStudio (0.99.875)           |
|language |(EN)                         |
|collate  |en_US.UTF-8                  |
|tz       |Europe/Amsterdam             |
|date     |2016-07-23                   |

## Packages

|package |*  |version |date       |source                       |
|:-------|:--|:-------|:----------|:----------------------------|
|curl    |   |1.0     |2016-07-23 |local (NA/NA@NA)             |
|knitr   |   |1.13.7  |2016-07-23 |Github (yihui/knitr@89de8c0) |

# Check results
2 packages with problems

## data.table (1.9.6)
Maintainer: Matt Dowle <mattjdowle@gmail.com>  
Bug reports: https://github.com/Rdatatable/data.table/issues

1 error  | 0 warnings | 1 note 

```
checking tests ... ERROR
Running the tests in ‘tests/tests.R’ failed.
Last 13 lines of output:
  > y = with(DT, eval(ll)) 
  First 6 of 652 :[1] 733 986 206 451 879 336
  forder decreasing argument test: seed = 1469269595  colorder = 5,4,3,2,1 
  Tests 1372.3+ not run. If required call library(GenomicRanges) first.
  Tests 1441-1444 not run. If required install the 'fr_FR.utf8' locale.
  
  Error in eval(expr, envir, enclos) : 
    18 errors out of 4390 (lastID=1557.4, endian=little, sizeof(long double)==16) in inst/tests/tests.Rraw on Sat Jul 23 12:26:41 2016. Search tests.Rraw for test numbers: 1253.224, 1253.226, 1253.228, 1253.23, 1253.264, 1253.266, 1253.312, 1253.314, 1253.316, 1253.318, 1253.36, 1253.362, 1253.364, 1253.366, 1253.456, 1253.458, 1253.46, 1253.462.
  Calls: test.data.table -> sys.source -> eval -> eval
  In addition: Warning message:
  In library(package, lib.loc = lib.loc, character.only = TRUE, logical.return = TRUE,  :
    there is no package called 'GenomicRanges'
  Execution halted

checking package dependencies ... NOTE
Package suggested but not available for checking: ‘GenomicRanges’
```

## rversions (1.0.2)
Maintainer: Gabor Csardi <csardi.gabor@gmail.com>  
Bug reports: https://github.com/metacran/rversions/issues

0 errors | 1 warning  | 0 notes

```
checking examples ... WARNING
Found the following significant warnings:

  Warning: 'xml_find_one' is deprecated.
  Warning: 'xml_find_one' is deprecated.
  Warning: 'xml_find_one' is deprecated.
  Warning: 'xml_find_one' is deprecated.
  Warning: 'xml_find_one' is deprecated.
  Warning: 'xml_find_one' is deprecated.
Deprecated functions may be defunct as soon as of the next release of
R.
See ?Deprecated.
```

