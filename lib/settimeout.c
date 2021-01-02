#include "common.h"
#include "utils.h"
#include <limits.h>
#include <signal.h>
#include "assert.h"
#include <sys/time.h>

typedef struct SetTimeoutState
{
  fn_setTimeoutCallback cb;
  void *arg;
  int64 triggerAt;
  struct SetTimeoutState *next;
} SetTimeoutState;

static SetTimeoutState *timeouts;
static SetTimeoutState *timeoutsBacklog;

static int64 epoch;
static int64 nextTimeoutAt;

static bool isTimeoutTriggering;

static bool didInit = false;

internal void init(void);
internal void triggerTimeout(void);
internal void handler_SIGALRM(int sig);

void ut_setTimeout(fn_setTimeoutCallback cb, void *arg, int64 ms)
{
  if (!didInit)
  {
    init();
  }
  assert(ms >= 0);

  SetTimeoutState *s = (SetTimeoutState *) malloc(sizeof(SetTimeoutState));
  s->cb        = cb;
  s->arg       = arg;
  s->triggerAt = ms + ut_timestampMS();
  s->next      = NULL;

  SetTimeoutState **p = isTimeoutTriggering? &timeoutsBacklog: &timeouts;

  s->next = *p;
  *p      = s;

  if (s->triggerAt < nextTimeoutAt)
  {
    nextTimeoutAt = s->triggerAt;

    struct itimerval itimer;
    memset(&itimer, 0, sizeof(struct itimerval));
    itimer.it_value.tv_sec  = ms / 1000;
    itimer.it_value.tv_usec = MAX((ms % 1000) * 1000, 1);
    setitimer(ITIMER_REAL, &itimer, NULL);
  }
}

internal void init(void)
{
  if (!didInit)
  {
    didInit             = true;
    timeouts            = NULL;
    timeoutsBacklog     = NULL;
    isTimeoutTriggering = false;
    epoch               = ut_timestampMS();
    nextTimeoutAt       = LONG_MAX;

    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handler_SIGALRM;
    sa.sa_flags   = 0;
    if (sigaction(SIGALRM, &sa, NULL) == -1)
    {
      ut_merror("sigaction");
      exit(1);
    }
  }
}

internal void handler_SIGALRM(int32 sig)
{
  triggerTimeout();
}

internal void triggerTimeout(void)
{
  if (!didInit)
  {
    init();
  }

  if (isTimeoutTriggering)
  {
    return;
  }
  isTimeoutTriggering = true;


  //1. trigger those timeouts that have expired
  {
    SetTimeoutState *head = timeouts;
    SetTimeoutState *prev = NULL;
    int64 now            = ut_timestampMS();

    while (head)
    {
      if (now >= head->triggerAt)
      {
        if (prev)
        {
          //trigger and behead from middle of list
          head->cb(head->arg);
          prev->next = head->next;
          free(head);
          head = prev->next;
        }
        else
        {
          //trigger and behead from top of list
          head->cb(head->arg);
          timeouts = head->next;
          free(head);
          head = timeouts;
        }

        //head->cb(head->arg) call may take much time to return
        now = ut_timestampMS();
      }
      else
      {
        prev = head;
        head = head->next;
      }
    }
  }

  //2. move timeouts from backlog to main list
  //   that may have been added by additional
  //   setTimeout requests during 1.
  {
    SetTimeoutState *temp;

    while (timeoutsBacklog)
    {
      temp            = timeoutsBacklog;
      timeoutsBacklog = timeoutsBacklog->next;
      temp->next      = timeouts;
      timeouts        = temp;
    }
  }

  //3. set next timeout
  {
    SetTimeoutState *head = timeouts;
    if (head)
    {
      int64 nextTimeoutAt_ = LONG_MAX;
      int64 now            = ut_timestampMS();

      while (head)
      {
        nextTimeoutAt_ = MIN(nextTimeoutAt_, head->triggerAt);
        head           = head->next;
      }
      int64 ms      = nextTimeoutAt_ - now; 
      ms            = MAX(ms, 0);
      nextTimeoutAt = now + ms;

      struct itimerval itimer;
      memset(&itimer, 0, sizeof(struct itimerval));
      itimer.it_value.tv_sec  = ms / 1000;
      itimer.it_value.tv_usec = MAX((ms % 1000) * 1000, 1);
      setitimer(ITIMER_REAL, &itimer, NULL);
    }
    else
    {
      nextTimeoutAt = LONG_MAX;
    }
  }

  isTimeoutTriggering = false;
}
