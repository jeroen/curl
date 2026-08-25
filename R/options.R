#' Listing curl features and options
#'
#' @description The functions `curl_version()` and `curl_options()` show the available
#' features, protocols and options supported by the local version of libcurl.
#'
#' @description You can use `curl_options_table()` to lookup the type for each
#' option: most options take a string, number, or `TRUE`/`FALSE` value, but some
#' options need a special enum/bitmask value: these are listed in the sections below.
#'
#' @export
#' @rdname curl_options
#' @param filter string: only return options with string in name
#' @examples # Available options
#' curl_options()
#'
#' # List proxy options
#' curl_options("proxy")
curl_options <- function(filter = ""){
  option_type_table <- make_option_type_table()
  opts <- structure(option_type_table$value, names = option_type_table$name)
  m <- grep(filter, names(opts), ignore.case = TRUE)
  opts[m]
}

#' @export
#' @rdname curl_options
curl_options_table <- function(filter = ""){
  option_type_table <- make_option_type_table()
  m <- grep(filter, option_type_table$name, ignore.case = TRUE)
  option_type_table[m,]
}

#' @useDynLib curl R_option_types
make_option_type_table <- local({
  cache <- NULL
  function(){
    if(is.null(cache)){
      out <- .Call(R_option_types)
      if(!length(out)) return(out)
      out$name <- tolower(out$name)
      out$type <- factor(out$type, levels = 0:8, labels = c("long", "values", "off_t",
        "object", "string", "slist", "cbptr", "blob", "function"))
      cache <<- structure(out, class = 'data.frame', row.names = seq_along(out$name))
    }
    return(cache)
  }
})
