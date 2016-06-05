.onLoad <- function(libname, pkgname){
  if (!grepl("mingw", R.Version()$platform))
    return()

  # Enable SSL on Windows if CA bundle is available (R >= 3.2.0)
  bundle <- Sys.getenv("CURL_CA_BUNDLE",
    file.path(R.home("etc"), "curl-ca-bundle.crt"))
  if (bundle != "" && file.exists(bundle)) {
    set_bundle(bundle)
  }
}

.onAttach <- function(libname, pkgname){
  if (grepl("mingw", R.Version()$platform) && !file.exists(get_bundle())){
    packageStartupMessage("No CA bundle found. SSL validation disabled.")

    # CRAN does not like warnings for r-oldrel. Fix once R 3.3 is out:
    #warning("No CA bundle found. SSL validation disabled.", call. = FALSE)
  }

  # Check for GnuTLS on Ubuntu
  if(identical(Sys.info()[["sysname"]], "Linux")){
    if(grepl("GnuTLS", curl_version()$ssl_version) && grepl("Debian|Ubuntu", utils::sessionInfo()$running)){
      packageStartupMessage("This version of curl was compiled against libcurl4-gnutls-dev which is known to have https issues.
It is recommended to install libcurl4-openssl-dev and recompile the 'curl' package in R.")
    }
  }
}

#' @useDynLib curl R_set_bundle
set_bundle <- function(path){
  .Call(R_set_bundle, path)
}

#' @useDynLib curl R_get_bundle
get_bundle <- function(){
  .Call(R_get_bundle)
}
