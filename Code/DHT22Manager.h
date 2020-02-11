#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

// ====================T AND H SENSOR DEFINITIONS======================
#define TH_SENSOR_DDR   DDRC
#define TH_SENSOR_PORT  PORTC
#define TH_SENSOR_PIN   PINC
#define TH_SENSOR_PCMSK PCMSK1
#define TH_SENSOR_PCIE  PCIE1

// All time is defined in MICRO seconds.

#define TH_SENSOR_UNSLEEP_TIME 18000

#define TH_SENSOR_REL_TIME 20

#define MIN_INIT_RESP_TIME 75
#define MAX_INIT_RESP_TIME 85

#define MIN_DATA_RESP_TIME 48
#define MAX_DATA_RESP_TIME 55
#define MIN_LOW_DATA_RESP_TIME 22
#define MAX_LOW_DATA_RESP_TIME 30
#define MIN_HIGH_DATA_RESP_TIME 68
#define MAX_HIGH_DATA_RESP_TIME 75  

#define TEMP_DATA_SIZE     16
#define HUMIDITY_DATA_SIZE 16
#define CHECK_DATA_SIZE    8

#define NUMBER_OF_DHT_DATA_BYTES 40

#define TEMPERATURE_SIGN_BIT    15
#define TEMPERATURE_NUMBER_MASK 0x7FFF

#define GET_INTEGER_PART(UInteger) ((UInteger / 10))
#define GET_FRACTAL_PART(UInteger) ((UInteger % 10))


// ====================T AND H SENSOR DEFINITIONS END======================

#include <avr/io.h>

#include "generalMacroses.h"
#include "timer0Manager.h"

// TODO remove unused states
typedef enum DhtManagerStateEnum 
{
  // TODO remove unused states
  DHT_NOT_INITIATED = 0,
  DHT_INACTIVE      = 1,
  UNSLEEPING_DHT    = 2,
  READING_DHT_DATA  = 3,
  CONVERTING_DATA   = 4,
  DATA_READY        = 5
} DhtManagerState;

typedef enum DhtReadingPhaseEnum
{
  // TODO remove unused states (if there are such)
  NOT_STARTED               = 0,
  RECEIVING_ACK             = 1,
  FETCHING_DATA             = 2
}
DhtReadingPhase;

typedef enum SignEnum
{
  NEG = 0,
  POS = 1
} Sign;

typedef struct TemperatureStruct
{
  uint8_t  receivedBitsNum;
  uint16_t temperData;
} Temperature;

typedef struct HumidityStruct
{
  uint8_t  receivedBitsNum;
  uint16_t humidData;
} Humidity;

typedef struct ValidationDataStruct
{
  uint8_t receivedBitsNum;
  uint8_t validationByte; 
} ValidationData;

typedef struct DHT22MStruct
{
  DhtManagerState state;
  DhtReadingPhase readPhase;
  Temperature     temper;
  Humidity        humid;
  ValidationData  valDat;
  uint32_t        lastFixedTime;
  uint8_t         pinNumber;
  uint8_t         dhtRespsTimings[NUMBER_OF_DHT_DATA_BYTES];
  uint8_t         numOfTimingsReceived;
} DHT22M; 

void DHT22M_init(DHT22M* , uint8_t pinNum);

ResultCode DHT22M_getData(DHT22M* dht22m, 
                          Temperature* temper, 
                          Humidity* humidity,
                          Timer0Manager* t0m);

ResultCode DHT22M_onPCInt(DHT22M* dht22m, Timer0Manager* t0m);
