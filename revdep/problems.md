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
|date     |2017-04-12                   |

## Packages

|package |*  |version |date       |source           |
|:-------|:--|:-------|:----------|:----------------|
|curl    |   |2.5     |2017-04-12 |local (NA/NA@NA) |

# Check results
1 packages with problems

## tidyquant (0.5.0)
Maintainer: Matt Dancho <mdancho@business-science.io>  
Bug reports: https://github.com/business-science/tidyquant/issues

1 error  | 0 warnings | 0 notes

```
checking tests ... ERROR
  Running ‘testthat.R’ [10s/38s]
Running the tests in ‘tests/testthat.R’ failed.
Last 13 lines of output:
  
  
  testthat results ================================================================
  OK: 183 SKIPPED: 2 FAILED: 8
  1. Failure: Test returns tibble with correct rows and columns. (@test_tq_get_exchange_rates.R#20) 
  2. Failure: Test returns tibble with correct rows and columns. (@test_tq_get_exchange_rates.R#21) 
  3. Failure: Test returns tibble with correct rows and columns. (@test_tq_get_exchange_rates.R#25) 
  4. Failure: Test returns tibble with correct rows and columns. (@test_tq_get_exchange_rates.R#27) 
  5. Failure: Test returns tibble with correct rows and columns. (@test_tq_get_metal_prices.R#20) 
  6. Failure: Test returns tibble with correct rows and columns. (@test_tq_get_metal_prices.R#21) 
  7. Failure: Test returns tibble with correct rows and columns. (@test_tq_get_metal_prices.R#25) 
  8. Failure: Test returns tibble with correct rows and columns. (@test_tq_get_metal_prices.R#27) 
  
  Error: testthat unit tests failed
  Execution halted
```

