int R_curl_callback_progress(SEXP fun, double dltotal, double dlnow,
  double ultotal, double ulnow);
size_t R_curl_callback_read(char *buffer, size_t size, size_t nitems, SEXP fun);
int R_curl_callback_debug(CURL *handle, curl_infotype type_, char *data,
                          size_t size, SEXP fun);
int default_callback_progress(void *dummy, double dltotal,
                              double dlnow, double ultotal, double ulnow);
