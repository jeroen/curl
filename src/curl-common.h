#include <Rinternals.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
  unsigned char *buf;
  size_t size;
} memory;

typedef struct {
  CURLM *m;
  struct refnode *list;
} multiref;

typedef struct {
  multiref *mref;
  struct refnode *node;
  memory content;
  SEXP complete;
  SEXP error;
} async;

typedef struct {
  SEXP handleptr;
  CURL *handle;
  struct curl_httppost *form;
  struct curl_slist *headers;
  memory resheaders;
  async async;
  int refCount;
  int locked;
} reference;

struct refnode {
  struct refnode *prev;
  struct refnode *next;
  reference *ref;
};

CURL* get_handle(SEXP ptr);
reference* get_ref(SEXP ptr);
void assert(CURLcode res);
void massert(CURLMcode res);
void stop_for_status(CURL *http_handle);
SEXP slist_to_vec(struct curl_slist *slist);
struct curl_slist* vec_to_slist(SEXP vec);
struct curl_httppost* make_form(SEXP form);
void set_form(reference *ref, struct curl_httppost* newform);
void set_headers(reference *ref, struct curl_slist *newheaders);
void reset_resheaders(reference *ref);
void clean_handle(reference *ref);
size_t push_disk(void* contents, size_t sz, size_t nmemb, FILE *ctx);
size_t append_buffer(void *contents, size_t sz, size_t nmemb, void *ctx);
CURLcode curl_perform_with_interrupt(CURL *handle);
int pending_interrupt();
SEXP make_handle_response(reference *ref);

/* refnode.c */
struct refnode *refnode_init();
struct refnode *refnode_add(struct refnode *head, reference *ptr);
reference * refnode_remove(reference * ref);
