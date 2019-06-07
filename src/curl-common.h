#include <Rinternals.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <stdlib.h>

#if LIBCURL_VERSION_MAJOR > 7 || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR >= 28)
#define HAS_MULTI_WAIT 1
#endif

typedef struct {
  unsigned char *buf;
  size_t size;
} memory;

typedef struct {
  SEXP multiptr;
  SEXP handles;
  CURLM *m;
} multiref;

typedef struct {
  multiref *mref;
  struct refnode *node;
  memory content;
  SEXP complete;
  SEXP error;
  SEXP data;
} async;

typedef struct {
  SEXP handleptr;
  CURL *handle;
  struct curl_httppost *form;
  struct curl_slist *headers;
  struct curl_slist *custom;
  char errbuf[CURL_ERROR_SIZE];
  memory resheaders;
  async async;
  int refCount;
  int locked;
} reference;

CURL* get_handle(SEXP ptr);
reference* get_ref(SEXP ptr);
void assert_status(CURLcode res, reference *ref);
void assert(CURLcode res);
void massert(CURLMcode res);
void stop_for_status(CURL *http_handle);
SEXP slist_to_vec(struct curl_slist *slist);
struct curl_slist* vec_to_slist(SEXP vec);
struct curl_httppost* make_form(SEXP form);
void set_form(reference *ref, struct curl_httppost* newform);
void reset_resheaders(reference *ref);
void reset_errbuf(reference *ref);
void clean_handle(reference *ref);
size_t push_disk(void* contents, size_t sz, size_t nmemb, FILE *ctx);
size_t append_buffer(void *contents, size_t sz, size_t nmemb, void *ctx);
size_t data_callback(void * data, size_t sz, size_t nmemb, SEXP fun);
CURLcode curl_perform_with_interrupt(CURL *handle);
int pending_interrupt();
SEXP make_handle_response(reference *ref);

/* reflist.c */
SEXP reflist_init();
SEXP reflist_add(SEXP x, SEXP target);
SEXP reflist_remove(SEXP x, SEXP target);
