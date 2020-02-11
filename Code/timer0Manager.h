#ifndef TIMER_0_MANAGER_H
#define TIMER_0_MANAGER_H
#include <stdint.h>

typedef enum TimerStateEnum 
{
  NOT_INITIATED = 0,
  INACTIVE      = 1,
  RUNNING       = 2
} TimerState;

typedef enum TimerUnitEnum
{
  S       = 0,
  MILLI_S = 1,
  MICRO_S = 2
} TimerUnit;

typedef enum TimerModeEnum 
{
  NORMAL = 0,
  CTC    = 2
} TimerMode;

typedef struct Timer0ManagerStruct
{
  TimerMode  mode;
  TimerState state;
  TimerUnit  unit;  
  uint8_t    step;
  uint32_t   currentTime;
  uint16_t   prescaler;
  uint8_t    numOfUsers;
} Timer0Manager;

ResultCode T0M_init(Timer0Manager* timerManagerToinit,
                    TimerMode mode,
                    TimerUnit unit,
                    uint8_t step);

ResultCode T0M_registerOneUser(Timer0Manager* tm);

ResultCode T0M_unregisterOneUser(Timer0Manager* tm);

inline TimerState T0M_getState(Timer0Manager* tm)
{
  // TODO consider checking of state and forming of reult code
  return tm->state;
}

inline TimerUnit T0M_getUnit(Timer0Manager* tm)
{
  // TODO consider checking of state and forming of reult code
  return tm->unit;
}

inline uint32_t T0M_getTime(Timer0Manager* tm)
{
  // TODO consider checking of state and forming of reult code
  return tm->currentTime;
}

inline void T0M_stepTime(Timer0Manager* tm)
{
  // TODO This function MUST remain inline, since it is crucial to have it fast running.
  // TODO consider checking of state and forming of reult code
  tm->currentTime += tm->step;
}

#endif