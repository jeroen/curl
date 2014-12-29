/*
  This macro brilliance is borrowed from Duncan Temple Lang.
  Thank you, Duncan!
*/

#undef CINIT
#define CINIT(a,b,c) {#a, CURLOPT_##a}

typedef struct {
  char name[40];
  int val;
} keyval;

keyval curl_options[] = {
#include "option_table.h"
};
