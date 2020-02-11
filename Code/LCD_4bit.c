#include <util/delay.h>

#include "LCD_4bit.h"
#include "generalMacroses.h"

// ======== LOW LEVEL FUNCTIONS. WORKING WITH PORTS/PINS/DDRS. =========

static void enableSignal()
{
  SET_BIT_TO_HIGH(LCD_PORT, E_PIN);
  _delay_us(2);
  SET_BIT_TO_LOW(LCD_PORT, E_PIN);
}

static inline void clearDataBus()
{
  LCD_PORT &= ~(DATA_BUS_HIGH << DATA_BUS_START_POS);
}

static inline void writeDataToDataBus(uint8_t data) 
{
  clearDataBus();
  LCD_PORT |= (data << DATA_BUS_START_POS);
}

static inline void dataBusToWriteMode()
{
  LCD_DDR |= (DATA_BUS_HIGH << DATA_BUS_START_POS);
}

static inline void rwToWriteMode()
{
  SET_BIT_TO_LOW(LCD_PORT, RW_PIN);
}

static inline void dataBusToReadMode()
{
  LCD_DDR &= ~(DATA_BUS_HIGH << DATA_BUS_START_POS);
}

static inline void rwToReadMode()
{
  SET_BIT_TO_HIGH(LCD_PORT, RW_PIN);
}

static inline void rsToInstructionMode()
{
  SET_BIT_TO_LOW(LCD_PORT, RS_PIN);
}

static inline void rsToDataMode()
{
  SET_BIT_TO_HIGH(LCD_PORT, RS_PIN);
}

// ======== HIGH LEVEL FUNCTIONS.=======================================


static void displayBusToWriteMode()
{
  dataBusToWriteMode();
  rwToWriteMode();
}

static inline void displayBusToReadMode()
{
  dataBusToReadMode();
  rwToReadMode();
}

static void writeHalfOfInstruct(uint8_t data)
{  	
  displayBusToWriteMode();
  
  rsToInstructionMode(); 
  
  writeDataToDataBus(data);	  
  
  enableSignal();   	
}

static void writeInstruction(uint8_t data)
{
  writeHalfOfInstruct(data >> FOUR_BITS);
  _delay_us(2);
  writeHalfOfInstruct(data &  FIRST_TETRADE_HIGH);
  _delay_us(100);	
}

static void writeSymbol(uint8_t symbol)
{
  rsToDataMode();
  displayBusToWriteMode();
  
  writeDataToDataBus(symbol >> FOUR_BITS);	  
  enableSignal();  
  
  _delay_us(2);
  
  writeDataToDataBus(symbol &  FIRST_TETRADE_HIGH);	  
  enableSignal();  
  
  _delay_us(100);
}

void LCD_InitFourBitBus()
{
  LCD_DDR |= (HIGH << RS_PIN) | (HIGH << RW_PIN) | (HIGH << E_PIN);

  _delay_ms(50); // init delay; required by datasheet
	
  writeHalfOfInstruct(0x2); // function set: 4-bit interface 
  _delay_us(50); // required by datasheet
  
  writeInstruction(0x28); // function set: 4-bit interface, 2 lines, 5x8 fornt
  _delay_us(50); // required by datasheet
	
  writeInstruction(0x0C); // display ON/OFF, disp ON, cursor OFF, blink OFF  
  _delay_us(50); // required by datasheet
	
  writeInstruction(0x01); // clear display
  _delay_ms(2); // required by datasheet
	
  writeInstruction(0x06); // entry mode set: cursor moves to right
}

static uint8_t displayIsBusy(uint8_t* cursorPos)
{
  uint8_t numOfRetries = 0;
  uint8_t result = 0;	  
  
  rsToInstructionMode();
  displayBusToReadMode();
  do
  {
	  SET_BIT_TO_HIGH(LCD_PORT, E_PIN);
    _delay_us(2);
    result = (LCD_PIN >> BF_PIN) & HIGH;
    *cursorPos = (LCD_PIN >> DATA_BUS_START_POS) & BITS_WITH_ADDR_COUNT_P1;
    SET_BIT_TO_LOW(LCD_PORT, E_PIN);
    _delay_us(1);   // required by datasheet
  
    SET_BIT_TO_HIGH(LCD_PORT, E_PIN);
    _delay_us(2);
    *cursorPos =  ((*cursorPos) << SIZE_OF_ADDR_HALF) 
                   | 
                  (LCD_PIN >> DATA_BUS_START_POS) & BITS_WITH_ADDR_COUNT_P2;
    SET_BIT_TO_LOW(LCD_PORT, E_PIN);    _delay_us(80);  // required by datasheet
    numOfRetries += 1;
  }
  while (result && numOfRetries < MAX_WRITE_RETRIES);
  
  return result;
}

uint8_t LCD_setDdramAddress(uint8_t address)
{
  uint8_t result = OK;  

  uint8_t addr;

  if (displayIsBusy(&addr))
  {	
    result = NOK;
  }
  else 
  {
    writeInstruction(DDRAM_BIT | address);
  }
  return result;
}

uint8_t LCD_writeSymbolToRam(uint8_t data)
{
  uint8_t result = OK;  
  uint8_t addr;  
  if (displayIsBusy(&addr))
  {	
    result = NOK;
  }
  else
  {
    writeSymbol(data);
  }
}

void LCD_writeErrorCode(uint8_t code)
{
  uint8_t cursorPos;
  displayIsBusy(&cursorPos);
  if (cursorPos >= SECOND_LINE_DDRAM_END_POS)
  {
    return;
  }
  LCD_writeSymbolToRam(code + 0x30);
  LCD_writeSymbolToRam(0xA0);

  cursorPos += 2;

  if (cursorPos >= FIRST_LINE_DDRAM_END_POS && cursorPos < SECOND_LINE_DDRAM_START_POS)
  {
    LCD_setDdramAddress(SECOND_LINE_DDRAM_START_POS);
  }
}

void LCD_writeUint(uint32_t number)
{
  uint8_t digitsToWrite[10];
  uint8_t numOfDigits = 0;

  do
  {
    digitsToWrite[numOfDigits] = number % 10;
    numOfDigits++;
  } while (number /= 10);

  do
  {
    numOfDigits --;
    LCD_writeSymbolToRam(digitsToWrite[numOfDigits] + 0x30);
  } while (numOfDigits);  
}

void LCD_writeString(const char *str)
{
	uint8_t data = 0;
	while (*str)
	{
		data = *str++;
		LCD_writeSymbolToRam(data);
	}
}
