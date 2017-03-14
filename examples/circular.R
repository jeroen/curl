library(curl)
h1 <- new_handle()
test <- function(){
  pool <- new_pool()
  h2 <- new_handle()
  cb <- function(...){}
  curl_fetch_multi('http://jeroen.github.io/images/frink.png', pool = pool, done = cb, handle = h1)
  curl_fetch_multi('http://jeroen.github.io/images/frink.png', pool = pool, done = cb, handle = h2)
  return(pool)
}

# Should clean 0 handles
pool <- test()
invisible(gc());

# Should clean 1 handle
multi_run(pool = pool)
invisible(gc());

# Should clean pool
rm(pool)
invisible(gc());

# should clean new pool + 2 handles
multi_run(pool = test())
rm(h1)
invisible(gc());

# Test circular GC problems
test2 <- function(){
  pool <- new_pool()
  cb <- function(...){}
  curl_fetch_multi('http://jeroen.github.io/images/frink.png', pool = pool, done = cb)
  curl_fetch_multi('http://jeroen.github.io/images/frink.png', pool = pool, done = cb)
}

# Should clean pool and both handles
test2()
invisible(gc())


# Test3 circular GC problems
test3 <- function(){
  pool <- new_pool()
  curl_fetch_multi('http://jeroen.github.io/images/frink.png', pool = pool)
  curl_fetch_multi('http://jeroen.github.io/images/frink.png', pool = pool)
  return(pool)
}

# Should clean pool and both handles
pool <- test3()
multi_list(pool)
rm(pool)
invisible(gc())


