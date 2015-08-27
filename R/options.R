option_table <- (function(){
  env <- new.env()
  source("tools/option_table.txt", env)
  option_table <- unlist(as.list(env))
  names(option_table) <- sub("^curlopt_", "", tolower(names(option_table)))
  return(option_table)
})()
