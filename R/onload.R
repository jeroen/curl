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
}

#' @useDynLib curl R_set_bundle
set_bundle <- function(path){
  .Call(R_set_bundle, path)
}

#' @useDynLib curl R_get_bundle
get_bundle <- function(){
  .Call(R_get_bundle)
}
