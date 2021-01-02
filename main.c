#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/select.h> 
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "lib/common.h"
#include "lib/utils.h"
#include "lib/session.h"

#define PORT 4000
#define LISTEN_BACKLOG 10

int main(int argc, char **argv)
{
  int32 listenSockFd;

  //1. listen
  {
    if ((listenSockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      ut_merror("socket");
      exit(1);
    }
    
    int sockoptEnable = 1;
    if (setsockopt(listenSockFd, SOL_SOCKET, SO_REUSEADDR, &sockoptEnable, sizeof(int)) == -1)
    {
      ut_merror("setsockopt");
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if ((bind(listenSockFd, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in))) == -1)
    {
      ut_merror("bind");
      exit(1);
    }

    if (listen(listenSockFd, LISTEN_BACKLOG) == -1)
    {
      ut_merror("listen");
      exit(1);
    }
    printf("Listening on %d\n", PORT);
  }

  //2. accept
  for(;;)
  {
    int result = accept(listenSockFd, NULL, NULL);
    if (result == -1 && errno == EINTR)
    {
      continue;
    }
    else if (result == -1)
    {
      ut_merror("accept");
      exit(1);
    }
    else
    {
      ss_onConnect(result);
    }
  }

  return 0;
}
