#include "animation_player.h"
#include "screen_reader.h"
#include "text_canvas.h"
#include "utils.h"

#define RUN_DURATION 285
#define INTRO_END_MARK 12

#define FRAME_COLS 100
#define FRAME_ROWS 38
#define FRAME_BUFFER_SIZE 4096

#define LYRICS_X    55
#define LYRICS_Y    15
#define LYRICS_COLS 45
#define LYRICS_ROWS 20

typedef struct AnimationPlayer
{
  int32 ranDuration;
  ScreenReader *screenReader;
  TextCanvas *textCanvas;

  char *scrollBuf;
  size_t scrollBufLen;
} AnimationPlayer;

static char *art_a1;
static size_t art_a1Len = FRAME_BUFFER_SIZE;

static char *art_a2;
static size_t art_a2Len = FRAME_BUFFER_SIZE;

static char *art_a3;
static size_t art_a3Len = FRAME_BUFFER_SIZE;

static char *art_a4;
static size_t art_a4Len = FRAME_BUFFER_SIZE;

static char *art_b1;
static size_t art_b1Len = FRAME_BUFFER_SIZE;

static char *art_b2;
static size_t art_b2Len = FRAME_BUFFER_SIZE;

static char *lyrics;
static size_t lyricsLen = FRAME_BUFFER_SIZE;


static bool didInit = false;

#define INIT() do { if (!didInit) { didInit = true; _init(); }  } while(0)
#define K(arg1, arg2) arg1 = (char *) malloc(arg2)
internal void _init(void)
{
  K(art_a1, art_a1Len);
  K(art_a2, art_a2Len);
  K(art_a3, art_a3Len);
  K(art_a4, art_a4Len);
  K(art_b1, art_b1Len);
  K(art_b2, art_b2Len);
  K(lyrics, lyricsLen);

  ut_readIntoBuffer("assets/ascii_art/art_1.txt",  art_a1, &art_a1Len);
  ut_readIntoBuffer("assets/ascii_art/art_2.txt",  art_a2, &art_a2Len);
  ut_readIntoBuffer("assets/ascii_art/art_3.txt",  art_a3, &art_a3Len);
  ut_readIntoBuffer("assets/ascii_art/art_4.txt",  art_a4, &art_a4Len);

  ut_readIntoBuffer("assets/ascii_art/glass_1.txt",  art_b1, &art_b1Len);
  ut_readIntoBuffer("assets/ascii_art/glass_2.txt",  art_b2, &art_b2Len);

  ut_readIntoBuffer("assets/lyrics.txt", lyrics, &lyricsLen);
}

AnimationPlayer *ap_createAnimationPlayer(void)
{
  INIT();

  AnimationPlayer *player = (AnimationPlayer *) malloc(sizeof(AnimationPlayer));
  player->ranDuration  = 0;
  player->screenReader = sr_createScreenReader(lyrics, lyricsLen, LYRICS_ROWS);
  player->textCanvas   = tc_createTextCanvas(FRAME_COLS, FRAME_ROWS, true);
  player->scrollBufLen = 2048;
  player->scrollBuf    = (char *) malloc(player->scrollBufLen);

  return player;
}

int32 ap_play(AnimationPlayer *player, void *outBuffer, size_t *outBufferLen)
{
  INIT();

  if (player->ranDuration >= RUN_DURATION)
  {
    return -1;
  }

  int32 R = ++player->ranDuration;

  char *art;
  size_t artLen;

  if (R < INTRO_END_MARK)
  {
    if (R <  (INTRO_END_MARK / 2))
    {
      art    = art_b1;
      artLen = art_b1Len;
    }
    else 
    {
      art    = art_b2;
      artLen = art_b2Len;
    }
  }
  else
  {
      switch(R % 4)
      {
        case 0:
        {
          art    = art_a1;
          artLen = art_a1Len;
          break;
        }
        case 1:
        {
          art    = art_a2;
          artLen = art_a2Len;
          break;
        }
        case 2:
        {
          art    = art_a3;
          artLen = art_a3Len;
          break;
        }
        case 3:
        {
          art    = art_a4;
          artLen = art_a4Len;
          break;
        }
      }
  }

  //1. blit art
  {
    Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.cols = FRAME_COLS;
    rect.rows = FRAME_ROWS;
    tc_blit(player->textCanvas, art, artLen, &rect);
  }

  //2. blit lyrics if beyond intro
  if (R >= INTRO_END_MARK)
  {
    size_t scrollBufLen2 = player->scrollBufLen;
    if (sr_scroll(player->screenReader, player->scrollBuf, &scrollBufLen2) != -1)
    {
      Rect rect;
      rect.x    = LYRICS_X;
      rect.y    = LYRICS_Y;
      rect.cols = LYRICS_COLS;
      rect.rows = LYRICS_ROWS;
      tc_blit(player->textCanvas, player->scrollBuf, scrollBufLen2, &rect);
    }
  }

  //3. write text canvas onto output buffer
  tc_readIntoBuffer(player->textCanvas, outBuffer, outBufferLen);

  tc_clearTextCanvas(player->textCanvas);

  return 0;
}

void ap_freeAnimationPlayer(AnimationPlayer *player)
{
  INIT();

  sr_freeScreenReader(player->screenReader);
  tc_freeTextCanvas(player->textCanvas);
  free(player->scrollBuf);
  free(player);
}
