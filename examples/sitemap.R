library(curl)
library(xml2)

stopifnot(packageVersion('xml2') >= 1.0)
stopifnot(packageVersion('curl') >= 2.0)

# Extracts hyperlinks from HTML page
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

crawl <- function(root, timeout = 300){
  pages <- new.env()
  pool <- curl::new_pool(total_con = 100, host_con = 100)

  crawl_page <- function(url){
    pages[[url]] <- NA
    h <- curl::new_handle(failonerror = TRUE, nobody = TRUE)
    curl_fetch_multi(url, handle = h, pool = pool, done = function(res){
      headers <- curl::parse_headers(res$headers)
      ctype <- headers[grepl("^content-type", headers, ignore.case = TRUE)]
      cat(sprintf("Found a %s file at: %s\n", ctype, url))
      if(isTRUE(grepl("text/html", ctype))){
        handle_setopt(h, nobody = FALSE, maxfilesize = 1e6)
        curl::curl_fetch_multi(url, handle = h, pool = pool, done = function(res){
          links = get_links(res)
          cat(sprintf("Done (%d): %s (%d links)\n", length(pages), url, length(links)))
          links <- grep(paste0("^", root), links, value = TRUE)
          pages[[url]] <- links
          lapply(links, function(href){
            if(is.null(pages[[href]]))
              crawl_page(href)
          })
        }, fail = function(errmsg){
          cat(sprintf("Fail: %s (%s)\n", url, errmsg))
        })
      }
    }, fail = function(errmsg){
      cat(sprintf("Fail: %s (%s)\n", url, errmsg))
    })
  }
  crawl_page(root)
  curl::multi_run(pool = pool, timeout = timeout)
  as.list(pages)
}

sitemap <- crawl(root = 'https://cloud.r-project.org')
