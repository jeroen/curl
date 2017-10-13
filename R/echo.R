#' Echo Service
#'
#' This function is only for testing purposes. It starts a local httpuv server to
#' echo the request body and content type in the response.
#'
#' @export
#' @param handle a curl handle object
#' @param port the port number on which to run httpuv server
#' @param progress show progress meter during http transfer
#' @param file path or connection to write body. Default returns body as raw vector.
#' @examples h <- handle_setform(new_handle(), foo = "blabla", bar = charToRaw("test"),
#' myfile = form_file(system.file("DESCRIPTION"), "text/description"))
#' formdata <- curl_echo(h)
#'
#' # Show the multipart body
#' cat(rawToChar(formdata$body))
#'
#' # Parse multipart
#' webutils::parse_http(formdata$body, formdata$content_type)
curl_echo <- function(handle, port = 9359, progress = interactive(), file = NULL){
  progress <- isTRUE(progress)
  formdata <- NULL
  if(!(is.null(file) || inherits(file, "connection") || is.character(file)))
    stop("Argument 'file' must be a file path or connection object")
  echo_handler <- function(env){
    if(progress){
      cat("\nRequest Complete!\n")
      progress <<- FALSE
    }

    formdata <<- as.list(env)
    http_method <- env[["REQUEST_METHOD"]]
    content_type <- env[["CONTENT_TYPE"]]
    type <- ifelse(length(content_type) && nchar(content_type), content_type, "empty")
    formdata$body <<- if(tolower(http_method) %in% c("post", "put")){
      if(!length(file)){
        env[["rook.input"]]$read()
      } else {
        write_to_file(env[["rook.input"]]$read, file)
      }
    }
    formdata[["rook.input"]] <<- NULL
    formdata[["rook.errors"]] <<- NULL
    names(formdata) <<- tolower(names(formdata))
    list(
      status = 200,
      body = "",
      headers = c("Content-Type" = "text/plain")
    )
  }

  # Start httpuv
  server_id <- httpuv::startServer("0.0.0.0", port, list(call = echo_handler))
  on.exit(httpuv::stopServer(server_id), add = TRUE)

  # Workaround bug in httpuv on windows that keeps protecting handler until next startServer()
  on.exit(rm(handle), add = TRUE)

  # httpuv 1.3.4 supports non-blocking service()
  waittime <- ifelse(utils::packageVersion('httpuv') > "1.3.3", NA, 1)

  # Post data from curl
  xfer <- function(down, up){
    if(progress){
      if(up[1] == 0 && down[1] == 0){
        cat("\rConnecting...")
      } else {
        cat(sprintf("\rUpload: %s (%d%%)   ", format_size(up[2]), as.integer(100 * up[2] / up[1])))
      }
    }
    # Need very low wait to prevent gridlocking!
    httpuv::service(waittime)
  }
  handle_setopt(handle, connecttimeout = 2, xferinfofunction = xfer, noprogress = FALSE)
  if(progress) cat("\n")
  curl_fetch_memory(paste0("http://localhost:", port, "/"), handle = handle)
  if(progress) cat("\n")
  return(formdata)
}

write_to_file <- function(readfun, filename){
  con <- if(inherits(filename, "connection")){
    filename
  } else {
    base::file(filename)
  }
  if(!isOpen(con)){
    open(con, "wb")
    on.exit(close(con))
  }
  while(length(buf <- readfun(1e6))){
    writeBin(buf, con)
  }
  return(filename)
}

format_size <- function(x){
  if(x < 1024)
    return(sprintf("%d b", x))
  if(x < 1048576)
    return(sprintf("%.2f Kb", x / 1024))
  return(sprintf("%.2f Mb", x / 1048576))
}
