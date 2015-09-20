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
#' # Sybol table
#' head(curl_symbols)
curl_options <- function(filter = ""){
  m <- grep(filter, fixed = TRUE, names(option_table))
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
