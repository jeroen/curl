#include <curl/curl.h>
void assert(CURLcode res);
void stop_for_status(CURL *http_handle);
CURL *make_handle(const char *url);
