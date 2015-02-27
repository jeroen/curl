#include <Windows.h>
#include <Winhttp.h>

void getProxyInfo(){
  WINHTTP_CURRENT_USER_IE_PROXY_CONFIG MyProxyConfig;
  if(!WinHttpGetIEProxyConfigForCurrentUser(&MyProxyConfig)){

  }
}

