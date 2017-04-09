#' Echo Service
#'
#' This function is only for testing purposes. It starts a local httpuv server to
#' echo the request body and content type in the response.
#'
#' @export
#' @param handle a curl handle object
#' @param port the port number on which to run httpuv server
#' @param progress show progress meter during http transfer
#' @examples h <- handle_setform(new_handle(), foo = "blabla", bar = charToRaw("test"),
#' myfile = form_file(system.file("DESCRIPTION"), "text/description"))
#' formdata <- curl_echo(h)
#'
#' # Show the multipart body
#' cat(rawToChar(formdata$body))
curl_echo <- function(handle, port = 9359, progress = interactive()){
  progress <- isTRUE(progress)
  formdata <- NULL
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
      env[["rook.input"]]$read()
    }
    formdata[["rook.input"]] <<- NULL
    list(
      status = 200,
      body = "",
      headers = c("Content-Type" = "text/plain")
    )
  }

  # Start httpuv
  server_id <- httpuv::startServer("0.0.0.0", port, list(call = echo_handler))
  on.exit(httpuv::stopServer(server_id), add = TRUE)
  httpuv::service()

  # Post data from curl
  handle_setopt(handle, connecttimeout = 2, xferinfofunction = function(down, up){
    if(progress){
      if(up[1] == 0 && down[1] == 0){
        cat("\rConnecting...")
      } else {
        cat(sprintf("\rUpload: %d (%d%%)", up[2], as.integer(100 * up[2] / up[1])))
      }
    }
    # Need very low wait to prevent gridlocking!
    httpuv::service(1)
  }, noprogress = FALSE)
  if(progress) cat("\n")
  curl_fetch_memory(paste0("http://localhost:", port, "/"), handle = handle)
  if(progress) cat("\n")
  return(formdata)
}
