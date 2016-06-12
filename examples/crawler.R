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
    sub("#.*", "", links)
  }, error = function(e){character()})
}

crawl <- function(startpage, timeout = 120){
  pages <- new.env()
  pool <- curl::new_pool(total_con = 20, host_con = 6)

  crawl_page <- function(url){
    h <- curl::new_handle(maxfilesize = 1e6)
    pages[[url]] <- NA
    curl::curl_fetch_multi(url, handle = h, pool = pool, done = function(res){
      if(res$status == 200){
        links = get_links(res)
        cat(sprintf("Done: %s (%d links)\n", url, length(links)))
        pages[[url]] = links
        lapply(links, function(href){
          if(is.null(pages[[href]]))
            crawl_page(href)
        })
      } else {
        cat(sprintf("Skipping page: %s (http %d)\n", url, res$status))
      }
    }, fail = function(errmsg){
      cat(sprintf("Fail: %s (%s)\n", url, errmsg))
    })
  }
  crawl_page(startpage)
  curl::multi_run(pool = pool, timeout = timeout)
}

crawl(start = 'http://www.nu.nl')
