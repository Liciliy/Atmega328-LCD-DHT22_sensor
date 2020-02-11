#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz clock speed, default value
#endif

#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

#include <avr/io.h>
#include "generalMacroses.h"
#include "timer0Manager.h"

const uint32_t US_TO_S_DIVIDER = 1000000;
const uint16_t MS_TO_S_DIVIDER = 1000;

const uint8_t ONE_USER = 1;
const uint8_t NO_USERS = 0;

// TODO add reInit function.

static ResultCode T0M_calculateOCR0X(TimerUnit unit, 
                                     uint8_t step, 
                                     uint8_t* calcResult)
{
  // TODO add checks for faulty data combinations: overflow, floating res values.
  // TODO add prescaler support.
  ResultCode result = RES_OK;

  uint32_t divider = 1;

  switch (unit)
  {
    case MICRO_S:
      divider = US_TO_S_DIVIDER;
    break;
  
    case MILLI_S:
      divider = MS_TO_S_DIVIDER;
    break;

    case S:
      divider = 1;
    break;

    default:
    break;
  }

  *calcResult = (step * F_CPU) / divider;
 
  return result;
}

ResultCode T0M_init(Timer0Manager* timerManagerToinit,
                    TimerMode mode,
                    TimerUnit unit,
                    uint8_t step)
{
  ResultCode result = RES_OK;
  // TODO add check if timer is already initiated if yes - return NOK.
  switch (mode)
  {
    case NORMAL:
    {
      // NOT SUPPORTED YET.
      result = RES_NOK;
    }      
    break;

    case CTC:
    {
      SET_BIT_TO_LOW(TCCR0A, WGM00);
      SET_BIT_TO_HIGH(TCCR0A, WGM01);
      SET_BIT_TO_LOW(TCCR0B, WGM02);
    }
    break;

    default:
      result = RES_NOK;
      break;
  }
  timerManagerToinit->mode = mode;
  timerManagerToinit->unit = unit;
  timerManagerToinit->step = step;
  timerManagerToinit->prescaler = 1;
  
  uint8_t valueToCompareWith = 0;

  // TODO process result code.
  T0M_calculateOCR0X(unit, step, &valueToCompareWith);

  OCR0A = valueToCompareWith;

  timerManagerToinit->state = INACTIVE;

  return result;
}
                
static void T0M_resetRegisters()
{
  TIMSK0 = 0;
  OCR0A  = 0;
  OCR0B  = 0;
  TCCR0B = 0;
  TCCR0A = 0;
  TCNT0  = 0;
  TIFR0  = 0;

}

ResultCode T0M_start(Timer0Manager* timerManagerToinit)
{
  // TODO add precaler support.
  ResultCode result = RES_OK;
  
  // TODO add check that global interrupt is enabled  
  if (timerManagerToinit->state == NOT_INITIATED ||
      timerManagerToinit->state == RUNNING)
  {
    result = RES_NOK;
  }
  else
  {
    SET_BIT_TO_HIGH(TIMSK0, OCIE0A); 

    uint8_t startSequence = 0;
    SET_BIT_TO_HIGH(startSequence, CS00);
    SET_BIT_TO_LOW(startSequence, CS01);
    SET_BIT_TO_LOW(startSequence, CS02);
    TCCR0B = startSequence;
    TCNT0 = 0;
    timerManagerToinit->currentTime = 0;
    timerManagerToinit->state = RUNNING;
  }  
  return result;  
}

ResultCode T0M_stop(Timer0Manager* timerManagerToinit)
{
  // TODO add precaler support.
  ResultCode result = RES_OK;
  
  if (timerManagerToinit->state == NOT_INITIATED ||
      timerManagerToinit->state == INACTIVE)
  {
    // TODO This if statement should be split into 2 parts with different 
    // res codes: NOT_INITIATED and INACTIVE
    result = RES_NOK;
  }
  else
  {
    TCNT0  = 0;
    TCCR0A = 0;
    TIMSK0 = 0;
    timerManagerToinit->currentTime = 0;
    timerManagerToinit->state = INACTIVE;
  }
   
  return result;
}

ResultCode T0M_registerOneUser(Timer0Manager* tm)
{
  ResultCode result = RES_OK;
  if (tm->state == NOT_INITIATED)
  {
    result = RES_NOK;
  }
  else
  {
    if (tm->state == INACTIVE)
    {
      T0M_start(tm);
    }
    tm->numOfUsers += ONE_USER;
  }

  return result;
}

ResultCode T0M_unregisterOneUser(Timer0Manager* tm)
{
  ResultCode result = RES_OK;
  
  if (tm->numOfUsers == 0 ||
      tm->state != RUNNING)
  {
    // TODO This if statement should be split into 2 parts with different 
    // res codes: TIMER_NOT_RUNNING and NO_USERS_REGISTERED
    result = RES_NOK;
  }
  else
  {
    tm->numOfUsers -= ONE_USER;
    if (tm->numOfUsers == NO_USERS)
    {
      T0M_stop(tm);
    }
  }
  
  return result;
}


