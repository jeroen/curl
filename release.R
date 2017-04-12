devtools::revdep_check_reset()
devtools::revdep_check(threads = 8, env_vars = c(NOT_CRAN=FALSE),
  ignore=c("data.table", "geoknife", "textreadr", "rgho"))
devtools::revdep_check_print_problems()
devtools::revdep_check_save_summary()
devtools::spell_check()
