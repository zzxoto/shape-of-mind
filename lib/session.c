#include "session.h"
#include "utils.h"
#include "animation_player.h"
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

//NOTE (Abhaya): Ideally 1000 if we were just streaming directly onto terminal
//998 seems good enough when streaming across LAN. This is very half baked. 
#define DISPATCH_INTERVAL_MS 998

//having these at begining of every animation frame, clears terminal
static uchar termCommands[]  = { 27, 91, 72, 27, 91, 74 }; 

typedef struct Session
{
  int32 sockfd;
  AnimationPlayer *animationPlayer;
  char *sendBuffer;
  size_t sendBufferLen;
} Session;

internal Session *createSession(int32 sockfd);
internal void freeSession(Session *session);
internal void routineDispatch(void *session);
internal int32 mysend(int32 sockfd, void *buffer, size_t len);

void ss_onConnect(int32 sockfd) 
{
  Session *s = createSession(sockfd);
  ut_setTimeout(routineDispatch, s, DISPATCH_INTERVAL_MS);
  printf("connected %d\n", sockfd);
}

internal Session *createSession(int32 sockfd)
{
  Session *session         = (Session *) malloc(sizeof(Session));
  session->sockfd          = sockfd;
  session->animationPlayer = ap_createAnimationPlayer();
  session->sendBufferLen   = 4096;
  session->sendBuffer      = (char *) malloc(session->sendBufferLen);

  return session;
}

internal void freeSession(Session *session)
{
  close(session->sockfd);
  ap_freeAnimationPlayer(session->animationPlayer);
  free(session);
}

internal void routineDispatch(void *session_)
{
  Session *session = (Session *) session_;

  //sendBuffer: sendBufferHead + sendBufferBody
  //sendBufferHead: stores clear terminal commands
  //sendBufferBody: stores animation frame
  
  //1. fill buffer head with clear terminal commands
  char *sendBufferHead = session->sendBuffer;
  size_t sendBufferHeadLen = sizeof(termCommands);
  for (int i = 0; i < sendBufferHeadLen; i++)
  {
    sendBufferHead[i] = termCommands[i];
  }

  //2. derive buffer body from buffer head
  char *sendBufferBody      = sendBufferHead + sendBufferHeadLen;
  ssize_t sendBufferBodyLen = session->sendBufferLen - sendBufferHeadLen;

  //3. copy animation frame onto buffer body
  if (ap_play(session->animationPlayer, sendBufferBody, &sendBufferBodyLen) == -1)
  {
      printf("played animation entirely for %d. Disconnecting.\n", session->sockfd);
      freeSession(session);
  }
  else
  {
    //4. tcp send
    if (mysend(session->sockfd, (void *) session->sendBuffer, sendBufferHeadLen +  sendBufferBodyLen) == -1)
    {
        printf("disconnected %d\n", session->sockfd);
        freeSession(session);
    }
    else
    {
      //5. schedule dispatch later
      ut_setTimeout(routineDispatch, session, DISPATCH_INTERVAL_MS);
    }
  }
}

static bool mysendInit = false;

//returns -1 if failed because of bad socket
//returns 0 if send succesfull
//if any other errors during sending, logs and exits program
internal int32 mysend(int32 sockfd, void *buffer, size_t len)
{
  if (!mysendInit)
  {
    mysendInit    = true;

    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags   = 0;

    if (sigaction(SIGPIPE, &sa, NULL) == -1)
    {
      ut_merror("sigaction");
    }
  }

  //TODO (Abhaya): blocking send() implementation, 
  //also doesn't check if `nSent < len` case as well.
  int32 nSent;
  do
  {
    nSent = send(sockfd, buffer, len, 0);
  } while(nSent == -1 && errno == EINTR);

  if (nSent == -1)
  {
    //NOTE (Abhaya): getsockopt sets 0 as error code, albeit 
    //error in send. Bug in low level implementation?
#if 0
    int errorCode;
    int errorCodeSize = sizeof(int);

    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &errorCode, &errorCodeSize);
    printf("(%d): %s\n", errorCode, strerror(errorCode));
#endif

    if (errno != EPIPE) 
    {
      ut_merror("send");
      exit(1);
    }

    return -1;
  }

  return 0;
}
