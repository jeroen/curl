## Requires dev version of curl!
# install.packages("https://github.com/jeroenooms/curl/archive/master.tar.gz", repos = NULL)
####

get_links <- function(res){
  tryCatch({
    headers <- curl::parse_headers(res$headers)
    ctype <- headers[grepl("^content-type", headers, ignore.case = T)]
    stopifnot(isTRUE(grepl("text/html", ctype)))
    doc <- xml2::read_html(res$content)
    nodes <- xml2::xml_find_all(doc, "//a[@href]")
    links <- xml2::xml_attr(nodes, "href")
    links <- xml2:::url_absolute(links, res$url)
    links <- grep("^https?://", links, value = TRUE)
    links <- sub("#.*", "", links)
    links <- sub("index.html$", "", links)
    unique(sub("/$", "", links))
  }, error = function(e){character()})
}

crawl <- function(startpage, timeout = 60, slots = 100){
  pages <- new.env()
  pool <- curl::new_pool(total_con = 50, host_con = 6)
  on.exit(rm(pool))

  crawl_page <- function(url){
    h <- curl::new_handle(maxfilesize = 1e6)
    pages[[url]] <- NA
    curl::curl_fetch_multi(url, handle = h, pool = pool, done = function(res){
      if(res$status == 200){
        links = get_links(res)
        cat(sprintf("Done (%d): %s (%d links)\n", length(pages), url, length(links)))
        pages[[url]] = links
        pending <- length(curl::multi_list(pool))
        if(pending < slots){
          links <- sample(links, min(5, length(links), slots - pending))
          lapply(links, function(href){
            if(is.null(pages[[href]]))
              crawl_page(href)
          })
        }
      } else {
        cat(sprintf("Skipping page: %s (http %d)\n", url, res$status))
      }
    }, fail = function(errmsg){
      cat(sprintf("Fail: %s (%s)\n", url, errmsg))
    })
  }
  crawl_page(startpage)
  curl::multi_run(pool = pool, timeout = timeout)
  return(pages)
}

system.time(pages <- crawl(startpage = 'https://news.ycombinator.com/'))
