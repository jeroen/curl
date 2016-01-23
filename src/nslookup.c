#include <Rinternals.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
#else
#include <netdb.h>
#include <arpa/inet.h>
#endif

SEXP R_nslookup(SEXP hostname) {
  /* The 'hints' arg is only needed for solaris */
  struct addrinfo hints;
  memset(&hints,0,sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = PF_UNSPEC;

  /* Because gethostbyname() is deprecated */
  struct addrinfo *addr;
  if(getaddrinfo(CHAR(STRING_ELT(hostname, 0)), "http", &hints, &addr))
    error("Failed to resolve hostname");

  /* For debugging */
  struct sockaddr *sa = addr->ai_addr;

  /* IPv4 vs v6 */
  char ip[INET6_ADDRSTRLEN];
  if (sa->sa_family == AF_INET) {
    struct sockaddr_in *sa_in = (struct sockaddr_in*) sa;
    inet_ntop(AF_INET, &(sa_in->sin_addr), ip, INET_ADDRSTRLEN);
  } else {
    struct sockaddr_in6 *sa_in = (struct sockaddr_in6*) sa;
    inet_ntop(AF_INET6, &(sa_in->sin6_addr), ip, INET6_ADDRSTRLEN);
  }
  freeaddrinfo(addr);
  return mkString(ip);
}
