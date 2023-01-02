#' List curl version and options.
#'
#' \code{curl_version()} shows the versions of libcurl, libssl and zlib and
#' supported protocols. \code{curl_options()} lists all options available in
#' the current version of libcurl.  The dataset \code{curl_symbols} lists all
#' symbols (including options) provides more information about the symbols,
#' including when support was added/removed from libcurl.
#'
#' @export
#' @param filter string: only return options with string in name
#' @examples # Available options
#' curl_options()
#'
#' # List proxy options
#' curl_options("proxy")
#'
#' # Symbol table
#' curl_symbols("proxy")
curl_options <- function(filter = ""){
  # First try new method: list run-time options
  opts <- option_type_table()
  if(length(opts)){
    m <- grep(filter, opts$name, ignore.case = TRUE)
    return(structure(opts$value[m], names = opts$name[m]))
  }
  curl_options_fallback(filter = filter)
}

curl_options_fallback <- function(filter = ""){
  # Fallback method: extracted from headers at build-time
  m <- grep(filter, names(option_table), ignore.case = TRUE)
  option_table[m]
}

option_table <- (function(){
  env <- new.env()
  if(file.exists("tools/option_table.txt")){
    source("tools/option_table.txt", env)
  } else if(file.exists("../tools/option_table.txt")){
    source("../tools/option_table.txt", env)
  } else {
    stop("Failed to find 'tools/option_table.txt' from:", getwd())
  }

  option_table <- unlist(as.list(env))
  names(option_table) <- sub("^curlopt_", "", tolower(names(option_table)))
  option_table[order(names(option_table))]
})()


#' This is only available for libcurl 7.73 and up.
#' @useDynLib curl R_option_types
option_type_table <- function(){
  out <- .Call(R_option_types)
  if(!length(out)) return(out)
  out$name <- tolower(out$name)
  out$type <- factor(out$type, levels = 0:8, labels = c("long", "values", "off_t",
    "object", "string", "slist", "cbptr", "blob", "function"))
  class(out) <- 'data.frame'
  return(out)
}
