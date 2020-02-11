#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz clock speed
#endif

#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "DHT22Manager.h"
#include "generalMacroses.h"
#include "LCD_4bit.h"
#include "timer0Manager.h"

//====================LED DEFINITIONS======================
#define LED_DDR  DDRC
#define LED_PORT PORTC
#define LED_PINS 0x03

#define FIRST_LED_PIN  0
#define SECOND_LED_PIN 1

#define ERROR_LED_DDR  DDRD
#define ERROR_LED_PORT PORTD
#define ERROR_LED_BIT  7
//====================LED DEFINITIONS END======================

void writeTemperatureAndHumidity(Temperature* temper, Humidity* humid)
{
  uint16_t data = temper->temperData;

  LCD_setDdramAddress(FIRST_LINE_DDRAM_START_POS);
  LCD_writeString("Temper.");

  LCD_setDdramAddress(FIRST_LINE_DDRAM_START_POS + 8);

  if (data >> TEMPERATURE_SIGN_BIT)  
    LCD_writeSymbolToRam(MINUS_SYMBOL);  
  else 
    LCD_writeSymbolToRam(PLUS_SYMBOL);

  data = data & TEMPERATURE_NUMBER_MASK;

  LCD_writeUint(       GET_INTEGER_PART(data));
  LCD_writeSymbolToRam(DOT_SYMBOL);
  LCD_writeUint(       GET_FRACTAL_PART(data));
  LCD_writeSymbolToRam(DEGREE_SYMBOL);
  LCD_writeSymbolToRam(C_LETTER_SYMBOL);

  data = humid->humidData;

  LCD_setDdramAddress(SECOND_LINE_DDRAM_START_POS);
  LCD_writeString("Humidity");

  LCD_setDdramAddress(SECOND_LINE_DDRAM_START_POS + 9);

  LCD_writeUint(       GET_INTEGER_PART(data));
  LCD_writeSymbolToRam(DOT_SYMBOL);
  LCD_writeUint(       GET_FRACTAL_PART(data));
  LCD_writeSymbolToRam(PROCENT_SYMBOL);
}

Timer0Manager tm;
DHT22M dht;

const uint32_t TIME_BETWEEN_MEASURMENTS = 3000000; 

int main(void)
{ 
  //================8kk freq set========= 
  CLKPR = 0b10000000;
  CLKPR = 0b00000000;
  //================8kk freq set END========= 

  //================ERROR LED INIT========= 
  ERROR_LED_DDR |=  (HIGH << ERROR_LED_BIT);   
  //================ERROR LED INIT END========= 

  //====================LED SET-UP======================
  LED_DDR |= LED_PINS;
  //====================LED SET-UP END======================

  //================DISPLAY INIT AND TEST CODE=========
  LCD_InitFourBitBus();
  
  LCD_setDdramAddress(FIRST_LINE_DDRAM_START_POS);  
  //================DISPLAY INIT AND TEST CODE END=========
  
  sei();
  T0M_init(&tm, CTC, MICRO_S, 7);
  DHT22M_init(&dht, 5);  
  
  //====================TEST BUTTON SET-UP======================
  //DDRB &= ~(1 << 0);
  //PORTB |= (1 << 0);
  //uint8_t oldPinB = PINB & (1 << 0);
  //DDRB |= (1 << 1);
  //uint8_t oldPinC = 0;
  //oldPinC = PINC;
  //====================TEST BUTTON SET-UP END======================

  uint8_t measure = 1;
  uint8_t sleepStarted = 0;
  Temperature temper;
  Humidity humidity;
  uint32_t sleepStartTime = 0;
  while(1)
  {    
    if (measure == 1)
    {      
      ResultCode reqRes = DHT22M_getData(&dht, &temper, &humidity, &tm);
      if (reqRes == RES_OK)
      {       
        LED_PORT ^= (1 << SECOND_LED_PIN);
        measure = 0;
        writeTemperatureAndHumidity(&temper, &humidity);
      }   
      else if (reqRes != RES_PROCESSING_REQUEST) 
      {
        LED_PORT ^= (1 << FIRST_LED_PIN);
      }  
    }
    if (measure == 0)
    {
      if (sleepStarted == 0)
      {
        LED_PORT ^= (1 << FIRST_LED_PIN);
        T0M_registerOneUser(&tm);
        sleepStartTime = tm.currentTime;
        sleepStarted = 1;
      }
      if (tm.currentTime - sleepStartTime > TIME_BETWEEN_MEASURMENTS)
      {
        ERROR_LED_PORT ^= (1 << ERROR_LED_BIT);
        sleepStartTime = 0;
        measure = 1;
        T0M_unregisterOneUser(&tm);
      }
    }
  }  
}

ISR(TIMER0_COMPA_vect)
{
  cli();        
  T0M_stepTime(&tm); 
  sei();
}


ISR(PCINT1_vect)
{    
  DHT22M_onPCInt(&dht, &tm);
}