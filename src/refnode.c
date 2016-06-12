#include "curl-common.h"

struct refnode * refnode_init() {
  struct refnode *x = calloc(1, sizeof(struct refnode));
  return x;
}

// Caller needs to treat return value as new head!
struct refnode * refnode_add(struct refnode *head, reference *ptr){
  struct refnode *newnode = refnode_init();
  newnode->ref = ptr;
  newnode->prev = head;
  head->next = newnode;
  ptr->async.node = newnode;
  return newnode;
}

// Tail node is recognized by x->ref == NULL
reference * refnode_remove(reference * ref){
  struct refnode *x = ref->async.node;
  if(x->prev)
    x->prev->next = x->next;
  if(x->next)
    x->next->prev = x->prev;
  if(ref->async.mref->list == x)
    ref->async.mref->list = x->prev; //update list head
  free(x);
  return ref;
}
