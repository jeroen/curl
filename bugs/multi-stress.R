# Random segfaults for curl 0.60 + openssl + nghttp2

library(curl)
pool <- new_pool()

while(TRUE){
  for(i in 1:500){
    tmp <- file.path(tempdir(), sprintf("file%d.html", i))
    curl::curl_fetch_multi('https://cloud.r-project.org', pool = pool, data = tmp)
  }
  cat("Running...\n")
  print(multi_run(pool = pool))
  cat("GC: ")
  gc()
  cat("OK!\n")
}
