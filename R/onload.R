.onLoad <- function(libname, pkgname){
  # Enable SSL on Windows if CA bundle is available (R >= 3.2.0)
  if(grepl("mingw", R.Version()$platform)){
    bundle <- Sys.getenv("CURL_CA_BUNDLE", file.path(Sys.getenv("R_Home"), "etc/curl-ca-bundle.crt"))
    if(file.exists(bundle)){
      set_bundle(bundle)
    } else {
      packageStartupMessage("No CA bundle found. SSL validation disabled.")
    }
  }
}

#' @useDynLib curl R_set_bundle
set_bundle <- function(path){
  .Call(R_set_bundle, path);
}
