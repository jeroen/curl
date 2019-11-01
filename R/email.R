#' Send email
#'
#' Use the curl SMTP client to send an email. The \code{message} argument must be
#' properly formatted \href{https://tools.ietf.org/html/rfc2822}{RFC2822} email message with From/To/Subject headers and CRLF
#' line breaks.
#'
#' @details
#'
#' @section Specifying the server, port, and protocol:
#'
#' The \code{smtp_server} argument takes a hostname, or an SMTP URL:
#'
#' \itemize{
#'   \item \code{mail.example.com} - hostname only
#'   \item \code{mail.example.com:587} - hostname and port
#'   \item \code{smtp://mail.example.com} - protocol and hostname
#'   \item \code{smtp://mail.example.com:587} - full SMTP URL
#'   \item \code{smtps://mail.example.com:465} - full SMTPS URL
#' }
#'
#' By default, the port will be 25, unless \code{smtps://} is specified--then the
#' default will be 465 instead. Note that port 25 is blocked on many networks
#' and that port 587 and \code{use_ssl=TRUE} is more typical for modern email
#' servers.
#'
#' @section Encrypting connections via SMTPS or STARTTLS:
#'
#' SMTP connections can be encrypted in one of two ways.
#'
#' If your email server listens on port 465, then use an
#' \code{smtps://hostname:465} URL. The SMTPS protocol \emph{guarantees} that
#' TLS will be used to protect your communications.
#'
#' If your email server listens on port 25 or 587, use an \code{smtp://} URL and
#' set \code{use_ssl = TRUE}. This uses STARTTLS to \emph{opportunistically} use
#' encryption (i.e. encryption will only be used if the server supports it).
#'
#' @export
#' @param mail_rcpt one or more recipient email addresses. Do not include names,
#' these go into the \code{message} headers.
#' @param mail_from email address of the sender.
#' @param message either a string or connection with (properly formatted) email
#' message, including sender/recipient/subject headers. See example.
#' @param smtp_server hostname or address of the SMTP server, or, an
#' \code{smtp://} or \code{smtps://} URL. See "Specifying the server, port,
#' and protocol" below.
#' @param use_ssl Attempt to encrypt the connection via STARTTLS, if the server
#' supports it. This is generally required for port 587.
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
                      use_ssl = NULL, verbose = TRUE, ...){
  if(grepl('://', smtp_server)) {
    # protocol was provided
    if (!grepl('^smtps?://', smtp_server)) {
      stop('smtp_server used an invalid protocol; only smtp:// and smtps:// are supported')
    }
    url <- smtp_server
  } else {
    if (grepl(":465$", smtp_server)) {
      url <- paste0('smtps://', smtp_server)
    } else {
      url <- paste0('smtp://', smtp_server)
    }
  }

  if (is.null(use_ssl)) {
    use_ssl <- grepl('^smtps://', url)
  }

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
  }, mail_from = mail_from, mail_rcpt = mail_rcpt, use_ssl = use_ssl,
      verbose = verbose, ...)
  curl_fetch_memory(url, handle = h)
}
