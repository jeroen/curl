#' Send email
#'
#' Use the curl SMTP client to send an email. The \code{message} argument must be
#' properly formatted RFC2822 email message with From/To/Subject headers and CRLF
#' line breaks.
#'
#' @export
#' @param mail_rcpt one or more recipient email addresses. Do not include names,
#' these go into the \code{message} headers.
#' @param mail_from email address of the sender.
#' @param message either a string or connection with (properly formatted) email
#' message, including sender/recipient/subject headers. See example.
#' @param smtp_server address of the SMTP server without the \code{smtp://} part
#' @param verbose print output
#' @param ... other options passed to \code{\link{handle_setopt}}. In most cases
#' you will need to set a \code{username} and \code{password} to authenticate
#' with the SMTP server.
#' @examples \donttest{# Message in RFC2822 format
#' message <- 'From: "Testbot" <jeroen@ocpu.io>
#' To: "Jeroen Ooms" <jeroenooms@gmail.com>
#' Subject: Hello there!
#'
#' Hi Jeroen,
#' I am sending this email using curl.'
#'
#'
#' # Actually send the email
#' recipients <- c('test@opencpu.org', 'jeroenooms@gmail.com')
#' sender <- 'test@ocpu.io'
#' username <- 'mail@ocpu.io'
#' password <- askpass::askpass(paste("SMTP server password for", username))
#' send_mail(sender, recipients, smtp_server = 'smtp.mailgun.org',
#'   message = message, username = username, password = password)}
send_mail <- function(mail_from, mail_rcpt, message, smtp_server = 'localhost',
                      verbose = TRUE, ...){
  if(is.character(message))
    message <- charToRaw(paste(message, collapse = '\r\n'))
  con <- if(is.raw(message)){
    rawConnection(message)
  } else if(inherits(message, "connection")){
    if(!isOpen(message))
      open(message, 'rb')
    message
  } else {
    stop("Body must be a string, raw vector, or connection object")
  }
  on.exit(close(con))
  total_bytes <- 0
  h <- new_handle(upload = TRUE, readfunction = function(nbytes, ...) {
    buf <- readBin(con, raw(), nbytes)
    total_bytes <<- total_bytes + length(buf)
    if(verbose){
      if(length(buf)){
        cat(sprintf("\rUploaded %d bytes...", total_bytes), file = stderr())
      } else {
        cat(sprintf("\rUploaded %d bytes... all done!\n", total_bytes), file = stderr())
      }
    }
    return(buf)
  }, mail_from = mail_from, mail_rcpt = mail_rcpt, verbose = verbose, ...)
  url <- paste0('smtp://', smtp_server)
  curl_fetch_memory(url, handle = h)
}
