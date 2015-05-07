int R_curl_callback_progress(SEXP fun, double dltotal, double dlnow,
  double ultotal, double ulnow);
size_t R_curl_callback_write(SEXP fun, size_t size, size_t nmemb,
  void *userdata);
size_t R_curl_callback_read(char *buffer, size_t size, size_t nitems, SEXP fun);
