#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "lib/common.h"
#include "lib/animation_player.h"

static uchar termCommands[]  = { 27, 91, 72, 27, 91, 74 }; 

int main(void)
{
  AnimationPlayer *player = ap_createAnimationPlayer();

  //displayBuffer = displayBufferHead + displayBufferBody
  //displayBufferHead: stores clear terminal commands
  //displayBufferBody: stores animation frame
  uchar displayBuffer[4096];

  uchar *displayBufferHead = displayBuffer;
  ssize_t displayBufferHeadLen = sizeof(termCommands);
  for (int i = 0; i < displayBufferHeadLen; i++)
  {
    displayBufferHead[i] = termCommands[i];
  }

  uchar *displayBufferBody     = displayBufferHead + displayBufferHeadLen;
  ssize_t displayBufferBodyLen = sizeof(displayBuffer) - displayBufferHeadLen;

  uchar *animationBuffer     = displayBufferBody;
  ssize_t animationBufferLen = displayBufferBodyLen;

  while (ap_play(player, animationBuffer, &animationBufferLen) != -1)
  {
    write(1, displayBuffer, displayBufferHeadLen + animationBufferLen);

    animationBufferLen = displayBufferBodyLen;

    sleep(1);
  }

  ap_freeAnimationPlayer(player);

  return 0;
}
