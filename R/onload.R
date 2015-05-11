.onLoad <- function(libname, pkgname){
  if (!grepl("mingw", R.Version()$platform))
    return()

  # Enable SSL on Windows if CA bundle is available (R >= 3.2.0)
  bundle <- Sys.getenv("CURL_CA_BUNDLE",
    file.path(R.home("etc"), "curl-ca-bundle.crt"))
  if (bundle != "" && file.exists(bundle)) {
    set_bundle(bundle)
  } else {
    warning("No CA bundle found. SSL validation disabled.", call. = FALSE)
  }
}

#' @useDynLib curl R_set_bundle
set_bundle <- function(path){
  .Call(R_set_bundle, path)
}
