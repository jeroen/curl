/* This file is autogenerated from typelist.h.in */
/* if headers are not included, we provide a copy */
#ifndef CURLINC_TYPECHECK_GCC_H

/* evaluates to true if option takes a long argument */
#define curlcheck_long_option(option)                   \
  (0 < (option) && (option) < CURLOPTTYPE_OBJECTPOINT)

#define curlcheck_off_t_option(option)          \
  ((option) > CURLOPTTYPE_OFF_T)

/* evaluates to true if option takes a data argument to pass to a callback */
#define curlcheck_cb_data_option(option)                                      \
  ((option) == /*CURLOPT_CHUNK_DATA*/ 10201 ||                                          \
   (option) == /*CURLOPT_CLOSESOCKETDATA*/ 10209 ||                                     \
   (option) == /*CURLOPT_DEBUGDATA*/ 10095 ||                                           \
   (option) == /*CURLOPT_FNMATCH_DATA*/ 10202 ||                                        \
   (option) == /*CURLOPT_HEADERDATA*/ 10029 ||                                          \
   (option) == /*CURLOPT_HSTSREADDATA*/ 10302 ||                                        \
   (option) == /*CURLOPT_HSTSWRITEDATA*/ 10304 ||                                       \
   (option) == /*CURLOPT_INTERLEAVEDATA*/ 10195 ||                                      \
   (option) == /*CURLOPT_IOCTLDATA*/ 10131 ||                                           \
   (option) == /*CURLOPT_OPENSOCKETDATA*/ 10164 ||                                      \
   (option) == /*CURLOPT_PREREQDATA*/ 10313 ||                                          \
   (option) == /*CURLOPT_PROGRESSDATA*/ 10057 ||                                        \
   (option) == /*CURLOPT_READDATA*/ 10009 ||                                            \
   (option) == /*CURLOPT_SEEKDATA*/ 10168 ||                                            \
   (option) == /*CURLOPT_SOCKOPTDATA*/ 10149 ||                                         \
   (option) == /*CURLOPT_SSH_KEYDATA*/ 10185 ||                                         \
   (option) == /*CURLOPT_SSL_CTX_DATA*/ 10109 ||                                        \
   (option) == /*CURLOPT_WRITEDATA*/ 10001 ||                                           \
   (option) == /*CURLOPT_RESOLVER_START_DATA*/ 10273 ||                                 \
   (option) == /*CURLOPT_TRAILERDATA*/ 10284 ||                                         \
   (option) == /*CURLOPT_SSH_HOSTKEYDATA*/ 10317 ||                                     \
   0)

/* evaluates to true if option takes a POST data argument (void* or char*) */
#define curlcheck_postfields_option(option)                                   \
  ((option) == /*CURLOPT_POSTFIELDS*/ 10015 ||                                          \
   (option) == /*CURLOPT_COPYPOSTFIELDS*/ 10165 ||                                      \
   0)

/* evaluates to true if option takes a struct curl_slist * argument */
#define curlcheck_slist_option(option)                                        \
  ((option) == /*CURLOPT_HTTP200ALIASES*/ 10104 ||                                      \
   (option) == /*CURLOPT_HTTPHEADER*/ 10023 ||                                          \
   (option) == /*CURLOPT_MAIL_RCPT*/ 10187 ||                                           \
   (option) == /*CURLOPT_POSTQUOTE*/ 10039 ||                                           \
   (option) == /*CURLOPT_PREQUOTE*/ 10093 ||                                            \
   (option) == /*CURLOPT_PROXYHEADER*/ 10228 ||                                         \
   (option) == /*CURLOPT_QUOTE*/ 10028 ||                                               \
   (option) == /*CURLOPT_RESOLVE*/ 10203 ||                                             \
   (option) == /*CURLOPT_TELNETOPTIONS*/ 10070 ||                                       \
   (option) == /*CURLOPT_CONNECT_TO*/ 10243 ||                                          \
   0)

#define curlcheck_string_option(x) \
(x > 10000 && x < 20000 && !curlcheck_slist_option(x) && !curlcheck_cb_data_option(x));

#endif
