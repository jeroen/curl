void assert(CURLcode res);
void stop_for_status(CURL *http_handle);
CURL *get_handle(SEXP ptr);
SEXP slist_to_vec(struct curl_slist *slist);
struct curl_slist* vec_to_slist(SEXP vec);
struct curl_httppost* make_form(SEXP form);
