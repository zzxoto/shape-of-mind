#include "common.h"

typedef struct AnimationPlayer AnimationPlayer;

AnimationPlayer *ap_createAnimationPlayer(void);
void ap_freeAnimationPlayer(AnimationPlayer *player);

//returns -1 if done playing, 0 if success and modifies outBuffer and outBufferLen
//
//If outBufferLen is less than what is required for storing anim. frame,
//then truncates the frame
int32 ap_play(AnimationPlayer *player, void *outBuffer, size_t *outBufferLen);
