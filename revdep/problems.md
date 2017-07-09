# Setup

## Platform

|setting  |value                        |
|:--------|:----------------------------|
|version  |R version 3.4.1 (2017-06-30) |
|system   |x86_64, darwin15.6.0         |
|ui       |X11                          |
|language |(EN)                         |
|collate  |en_US.UTF-8                  |
|tz       |Europe/Amsterdam             |
|date     |2017-07-09                   |

## Packages

|package |*  |version |date       |source                 |
|:-------|:--|:-------|:----------|:----------------------|
|curl    |   |2.8     |2017-07-09 |local (jeroen/curl@NA) |

# Check results

4 packages with problems

|package    |version | errors| warnings| notes|
|:----------|:-------|------:|--------:|-----:|
|biomartr   |0.5.1   |      1|        0|     0|
|data.table |1.10.4  |      1|        0|     0|
|stplanr    |0.1.8   |      1|        0|     0|
|tidyquant  |0.5.1   |      1|        0|     0|

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

## stplanr (0.1.8)
Maintainer: Robin Lovelace <rob00x@gmail.com>  
Bug reports: https://github.com/ropensci/stplanr/issues

1 error  | 0 warnings | 0 notes

```
checking examples ... ERROR
Running examples in ‘stplanr-Ex.R’ failed
The error most likely occurred in:

> base::assign(".ptime", proc.time(), pos = "CheckExEnv")
> ### Name: onewayid
> ### Title: Aggregate ods so they become non-directional
> ### Aliases: onewayid onewayid onewayid.data.frame onewayid
> ###   onewayid.SpatialLines
> 
> ### ** Examples
> 
> data(flow)
> flow_oneway = onewayid(flow, attrib = 3)
`.cols` has been renamed and is deprecated, please use `.vars`
Error in summarise_impl(.data, dots) : 
  Evaluation error: could not find function "nth".
Calls: onewayid ... summarise -> summarise.tbl_df -> summarise_impl -> .Call
Execution halted
```

## tidyquant (0.5.1)
Maintainer: Matt Dancho <mdancho@business-science.io>  
Bug reports: https://github.com/business-science/tidyquant/issues

1 error  | 0 warnings | 0 notes

```
checking tests ... ERROR
  Running ‘testthat.R’ [14s/45s]
Running the tests in ‘tests/testthat.R’ failed.
Last 13 lines of output:
  1. Error: Test prints warning message on invalid x input. (@test_tq_get_dividends.R#23) 
  2. Error: Test returns NA on invalid x input. (@test_tq_get_dividends.R#27) 
  3. Failure: Test returns tibble with correct rows and columns. (@test_tq_get_exchange_rates.R#25) 
  4. Failure: Test returns tibble with correct rows and columns. (@test_tq_get_metal_prices.R#26) 
  5. Failure: Test prints warning message on invalid x input. (@test_tq_get_splits.R#23) 
  6. Failure: Test 1 returns tibble with correct rows and columns. (@test_tq_get_stock_prices.R#19) 
  7. Failure: Test 2 returns tibble with correct rows and columns. (@test_tq_get_stock_prices.R#28) 
  
  Error: testthat unit tests failed
  In addition: Warning messages:
  1: Oanda only provides historical data for the past 180 days. Symbol: EUR/USD 
  2: Oanda only provides historical data for the past 180 days. Symbol: EUR/USD 
  3: Oanda only provides historical data for the past 180 days. Symbol: gold 
  4: Oanda only provides historical data for the past 180 days. Symbol: gold 
  Execution halted
```

