#include "DHT22Manager.h"

#include <util/delay.h>
#include "LCD_4bit.h"

inline void enablePCInterrupt()
{
  SET_BIT_TO_HIGH(PCICR, TH_SENSOR_PCIE);
}

inline void disablePCInterrupt()
{
  SET_BIT_TO_LOW(PCICR, TH_SENSOR_PCIE);
}

inline void releaseBus(DHT22M* dht22m)
{
  SET_BIT_TO_LOW(TH_SENSOR_DDR, dht22m->pinNumber); 
  SET_BIT_TO_HIGH(TH_SENSOR_PORT, dht22m->pinNumber);  
}

inline uint8_t allDataReceived(DHT22M* dht)
{
  if (dht->numOfTimingsReceived == NUMBER_OF_DHT_DATA_BYTES)
  {
    return TRUE;
  }
  else return FALSE;
}

ResultCode timingToBit(uint8_t timePassed, uint8_t timeStep)
{
  ResultCode res = RES_LOW;
  if (timePassed >= (MIN_LOW_DATA_RESP_TIME - (timeStep * 2))
      &&
      timePassed <= (MAX_LOW_DATA_RESP_TIME + timeStep))
  {
    res = RES_LOW;
  }
  else if (timePassed >= ((timeStep * 2))
           &&
           timePassed <= (MAX_HIGH_DATA_RESP_TIME + timeStep))
  {
    res = RES_HIGH;
  }
  else
  {
    res = RES_WRONG_DATA_RESP_TIMING;
  }
  
  return res;
}

ResultCode convertTimingsAndValidateData(DHT22M* dht, uint8_t timeStep)
{
  ResultCode res = RES_OK;
  uint8_t bitNum = 0;
  
  for (; bitNum < HUMIDITY_DATA_SIZE; bitNum++)
  {
    ResultCode elavRes = timingToBit(dht->dhtRespsTimings[bitNum], 
                                            timeStep);

    if (elavRes == RES_HIGH || elavRes == RES_LOW)
      dht->humid.humidData = (dht->humid.humidData << 1) | elavRes;
    else 
      res = elavRes;
  }

  for (; bitNum < HUMIDITY_DATA_SIZE + TEMP_DATA_SIZE; bitNum++)
  {
    ResultCode elavRes = timingToBit(dht->dhtRespsTimings[bitNum], 
                                            timeStep);

    if (elavRes == RES_HIGH || elavRes == RES_LOW)
      dht->temper.temperData = (dht->temper.temperData << 1) | elavRes;
    else 
      res = elavRes;
  }

  for (; bitNum < HUMIDITY_DATA_SIZE + TEMP_DATA_SIZE + CHECK_DATA_SIZE; bitNum++)
  {
    ResultCode elavRes = timingToBit(dht->dhtRespsTimings[bitNum], 
                                            timeStep);

    if (elavRes == RES_HIGH || elavRes == RES_LOW)
      dht->valDat.validationByte = (dht->valDat.validationByte << 1) | elavRes;
    else 
      res = elavRes;
  }

  return res;
}

void DHT22M_init(DHT22M* dht22m, uint8_t pinNum)
{
  dht22m->pinNumber = pinNum;
  _delay_ms(2000);	
  SET_BIT_TO_HIGH(TH_SENSOR_DDR,   dht22m->pinNumber);
  SET_BIT_TO_HIGH(TH_SENSOR_PORT,  dht22m->pinNumber);  
  SET_BIT_TO_HIGH(TH_SENSOR_PCMSK, dht22m->pinNumber);

  dht22m->state = DHT_INACTIVE;
}

ResultCode DHT22M_getData(DHT22M* dht22m, 
                          Temperature* temper, 
                          Humidity* humidity,
                          Timer0Manager* t0m)
{
  ResultCode result = RES_OK;
  if (dht22m->state == READING_DHT_DATA)
  { 
    // TODO Add chack that DHT did responce via two 80 us impulses.
    // This must be done by comparing current time and time of data request 
    // start.
    // NOTE: before reading of current time - interruptss must be disabled
    // in order to avoid threads problem (simultaniousa read of time here 
    // and change of it by timer interrupt).
    result = RES_PROCESSING_REQUEST;
  } 
  else if (dht22m->state == UNSLEEPING_DHT)
  {    
    if (BIT_IS_SET_TO_LOW(TH_SENSOR_PORT, dht22m->pinNumber)
       &&
       ((t0m->currentTime - dht22m->lastFixedTime) >= TH_SENSOR_UNSLEEP_TIME))
    {      
      releaseBus(dht22m);      
      dht22m->lastFixedTime = t0m->currentTime;
      dht22m->state = READING_DHT_DATA; 
      dht22m->readPhase = NOT_STARTED;
      enablePCInterrupt(); 
    }
    result = RES_PROCESSING_REQUEST;    
  }
  else if (dht22m->state == DHT_INACTIVE)
  {    
    // TODO check if last data fetch was more than 2 sec ago 
    // (hardware limitation).
    dht22m->state = UNSLEEPING_DHT;
    T0M_registerOneUser(t0m);
    dht22m->lastFixedTime = t0m->currentTime;
    SET_BIT_TO_LOW(TH_SENSOR_PORT, dht22m->pinNumber);

    result = RES_PROCESSING_REQUEST;
  }
  else if (dht22m->state == DHT_NOT_INITIATED)
  {
    result = RES_OBJECT_IN_WRONG_STATE;
  }
  else if (dht22m->state == CONVERTING_DATA)
  {
    result = convertTimingsAndValidateData(dht22m, t0m->step);
    // TODO perform data validation
    *temper   = dht22m->temper;
    *humidity = dht22m->humid;
    dht22m->state = DHT_INACTIVE;
    // TODO clear dht22m temper, hum and val data
    // TODO restor data bus state, to be ready for next transfer.
    T0M_unregisterOneUser(t0m);
  }
  return result;
}

ResultCode DHT22M_onPCInt(DHT22M* dht22m, Timer0Manager* t0m)
{
  ResultCode result = RES_OK;
 
  switch (dht22m->readPhase)
  {
    case FETCHING_DATA:
    {
      if (BIT_IS_SET_TO_LOW(TH_SENSOR_PIN, dht22m->pinNumber))
      {
        dht22m->dhtRespsTimings[dht22m->numOfTimingsReceived] = t0m->currentTime - dht22m->lastFixedTime;
        dht22m->numOfTimingsReceived++;
        if (allDataReceived(dht22m))
        {
          dht22m->readPhase = NOT_STARTED;
          dht22m->state     = CONVERTING_DATA;
          disablePCInterrupt();
        }
      }      
    }
    break;

    case NOT_STARTED:
    {  
      dht22m->readPhase = RECEIVING_ACK;
    }
    break;

    case RECEIVING_ACK:
    {      
      uint8_t timePassed = t0m->currentTime - dht22m->lastFixedTime;
      if ((timePassed < (MIN_INIT_RESP_TIME - t0m->step))
          ||
          (timePassed > (MAX_INIT_RESP_TIME + t0m->step)))
      {
        result = RES_WRONG_RESP_TIMING;
      }
      else if (BIT_IS_SET_TO_LOW(TH_SENSOR_PIN, dht22m->pinNumber))
      {
        dht22m->readPhase = FETCHING_DATA;
        dht22m->numOfTimingsReceived = 0;
      }        
    }    
    break;
    
    default:
      result = RES_NOK;
      break;
  }
   
  dht22m->lastFixedTime = t0m->currentTime;
  return result;
}