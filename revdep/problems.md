# Setup

## Platform

|setting  |value                        |
|:--------|:----------------------------|
|version  |R version 3.4.1 (2017-06-30) |
|system   |x86_64, darwin15.6.0         |
|ui       |RStudio (1.0.143)            |
|language |(EN)                         |
|collate  |en_US.UTF-8                  |
|tz       |Europe/Amsterdam             |
|date     |2017-07-18                   |

## Packages

|package |*  |version |date       |source                 |
|:-------|:--|:-------|:----------|:----------------------|
|curl    |   |2.8     |2017-07-18 |local (jeroen/curl@NA) |

# Check results

5 packages with problems

|package    |version | errors| warnings| notes|
|:----------|:-------|------:|--------:|-----:|
|biomartr   |0.5.1   |      1|        0|     0|
|data.table |1.10.4  |      1|        0|     0|
|osmdata    |0.0.4   |      1|        0|     0|
|tidyquant  |0.5.1   |      1|        0|     0|
|TTR        |0.23-2  |      1|        0|     0|

## biomartr (0.5.1)
Maintainer: Hajk-Georg Drost <hgd23@cam.ac.uk>  
Bug reports: https://github.com/HajkD/biomartr/issues

1 error  | 0 warnings | 0 notes

```
checking package dependencies ... ERROR
Package required but not available: ‘Biostrings’

See section ‘The DESCRIPTION file’ in the ‘Writing R Extensions’
manual.
```

## data.table (1.10.4)
Maintainer: Matt Dowle <mattjdowle@gmail.com>  
Bug reports: https://github.com/Rdatatable/data.table/issues

1 error  | 0 warnings | 0 notes

```
checking whether package ‘data.table’ can be installed ... ERROR
Installation failed.
See ‘/Users/jeroen/workspace/curl/revdep/checks/data.table.Rcheck/00install.out’ for details.
```

## osmdata (0.0.4)
Maintainer: Mark Padgham <mark.padgham@email.com>  
Bug reports: https://github.com/osmdatar/osmdata/issues

1 error  | 0 warnings | 0 notes

```
checking examples ... ERROR
Running examples in ‘osmdata-Ex.R’ failed
The error most likely occurred in:

> base::assign(".ptime", proc.time(), pos = "CheckExEnv")
> ### Name: opq_string
> ### Title: Convert an overpass query into a text string
> ### Aliases: opq_string opq_to_string
> 
> ### ** Examples
> 
> q <- opq ("hampi india")
Error in curl::curl_fetch_memory(url, handle = handle) : 
  Timeout was reached: Resolving timed out after 211654 milliseconds
Calls: opq ... request_fetch -> request_fetch.write_memory -> <Anonymous> -> .Call
Execution halted
```

## tidyquant (0.5.1)
Maintainer: Matt Dancho <mdancho@business-science.io>  
Bug reports: https://github.com/business-science/tidyquant/issues

1 error  | 0 warnings | 0 notes

```
checking tests ... ERROR
  Running ‘testthat.R’ [13s/41s]
Running the tests in ‘tests/testthat.R’ failed.
Last 13 lines of output:
  2: Oanda only provides historical data for the past 180 days. Symbol: EUR/USD 
  3: Oanda only provides historical data for the past 180 days. Symbol: gold 
  4: Oanda only provides historical data for the past 180 days. Symbol: gold 
  testthat results ================================================================
  OK: 159 SKIPPED: 2 FAILED: 7
  1. Error: Test prints warning message on invalid x input. (@test_tq_get_dividends.R#23) 
  2. Error: Test returns NA on invalid x input. (@test_tq_get_dividends.R#27) 
  3. Failure: Test returns tibble with correct rows and columns. (@test_tq_get_exchange_rates.R#25) 
  4. Failure: Test returns tibble with correct rows and columns. (@test_tq_get_metal_prices.R#26) 
  5. Failure: Test prints warning message on invalid x input. (@test_tq_get_splits.R#23) 
  6. Failure: Test 1 returns tibble with correct rows and columns. (@test_tq_get_stock_prices.R#19) 
  7. Failure: Test 2 returns tibble with correct rows and columns. (@test_tq_get_stock_prices.R#28) 
  
  Error: testthat unit tests failed
  Execution halted
```

## TTR (0.23-2)
Maintainer: Joshua Ulrich <josh.m.ulrich@gmail.com>  
Bug reports: https://github.com/joshuaulrich/TTR/issues

1 error  | 0 warnings | 0 notes

```
checking whether package ‘TTR’ can be installed ... ERROR
Installation failed.
See ‘/Users/jeroen/workspace/curl/revdep/checks/TTR.Rcheck/00install.out’ for details.
```

