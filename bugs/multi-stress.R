# Random segfaults for curl 0.60 + openssl + nghttp2

library(curl)
pool <- new_pool()
tlds <- Filter(function(x){nchar(x) < 3}, urltools::tld_dataset)


while(TRUE){
  server <- paste0('https://www.google.', sample(tlds, 1))
  cat("Adding: ", server, "\n")
  for(i in 1:100){
    curl::curl_fetch_multi(server, pool = pool)
  }
  cat("Running...\n")
  print(multi_run(pool = pool))
  cat("GC: ")
  gc()
  cat("OK!\n")
}
