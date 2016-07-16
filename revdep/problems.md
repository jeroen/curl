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
|date     |2016-07-16                   |

## Packages

|package   |*  |version |date       |source                       |
|:---------|:--|:-------|:----------|:----------------------------|
|curl      |   |1.0     |2016-07-16 |local (NA/NA@NA)             |
|knitr     |   |1.13.6  |2016-07-16 |Github (yihui/knitr@0652b6e) |
|rmarkdown |   |1.0     |2016-07-08 |cran (@1.0)                  |

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
  First 6 of 649 :[1] 931 546 577 121 398 146
  forder decreasing argument test: seed = 1468661457  colorder = 5,1,4,2,3 
  Tests 1372.3+ not run. If required call library(GenomicRanges) first.
  Tests 1441-1444 not run. If required install the 'fr_FR.utf8' locale.
  
  Error in eval(expr, envir, enclos) : 
    6 errors out of 4390 (lastID=1557.4, endian=little, sizeof(long double)==16) in inst/tests/tests.Rraw on Sat Jul 16 11:31:06 2016. Search tests.Rraw for test numbers: 1253.28, 1253.282, 1253.392, 1253.394, 1253.4, 1253.402.
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

