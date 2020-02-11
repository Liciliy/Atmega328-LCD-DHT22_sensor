#ifndef LCD_4BIT_H
#define LCD_4BIT_H

#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

#include <avr/io.h>

//====================LCD USER REQUIRED DEFINITIONS START============================
#define RS_PIN 0
#define RW_PIN 1
#define E_PIN  2

#define LCD_DDR  DDRD
#define LCD_PORT PORTD
#define LCD_PIN  PIND
#define DATA_BUS_START_POS 3
//====================LCD USER REQUIRED DEFINITIONS END========================


#define DATA_BUS_HIGH 0b00001111


#define BF_PIN (3 + DATA_BUS_START_POS)
#define MAX_WRITE_RETRIES 10
#define DDRAM_BIT 0x80

#define DISPLAY_LINE_SIZE 16
#define FIRST_LINE_DDRAM_START_POS 0x00
#define FIRST_LINE_DDRAM_END_POS 0x27
#define SECOND_LINE_DDRAM_START_POS 0x40
#define SECOND_LINE_DDRAM_END_POS 0x67

#define BITS_WITH_ADDR_COUNT_P1 0b0111
#define BITS_WITH_ADDR_COUNT_P2 0b1111
#define SIZE_OF_ADDR_HALF       4

//====================SYMBOLS DEFINITIONS START============================
#define MINUS_SYMBOL    0x2D
#define PLUS_SYMBOL     0x2B
#define DOT_SYMBOL      0x2E
#define DEGREE_SYMBOL   0xDF
#define C_LETTER_SYMBOL 0x43
#define PROCENT_SYMBOL  0x25
//====================SYMBOLS DEFINITIONS END============================


#ifndef RS_PIN 
#error RS_PIN macros is not defined. Please define the macros with a number of MC pin connected to LCD RS pin. EXAMPLE: #define RS_PIN 0
#endif

#ifndef RW_PIN
#error RW_PIN macros is not defined. Please define the macros with a number of MC pin connected to LCD R/W pin. EXAMPLE: #define RW_PIN 1
#endif

#ifndef E_PIN
#error E_PIN macros is not defined. Please define the macros with a number of MC pin connected to LCD E pin. EXAMPLE: #define E_PIN 2
#endif

#ifndef LCD_DDR
#error LCD_DDR macros is not defined. Please define the macros with a MC DDRx register responcible for connected LCD handling. EXAMPLE: #define LCD_DDR DDRD
#endif

#ifndef LCD_PIN
#error LCD_PIN macros is not defined. Please define the macros with a MC PINx register responcible for connected LCD handling. EXAMPLE: #define LCD_PIN PIND
#endif

#ifndef LCD_PORT
#error LCD_PORT macros is not defined. Please define the macros with a MC PORTx register responcible for connected LCD handling. EXAMPLE: #define LCD_PORT PORTD
#endif

#ifndef DATA_BUS_START_POS
#error DATA_BUS_START_POS macros is not defined. Please define the macros with a number of MC pin connected to LCD DB4 pin. EXAMPLE: #define DATA_BUS_START_POS 3 NOTE: DBx pins of LCD must be connected consecutively to the MC.
#endif

void LCD_InitFourBitBus();

uint8_t LCD_setDdramAddress(uint8_t address);

uint8_t LCD_writeSymbolToRam(uint8_t data);

void LCD_writeErrorCode(uint8_t code);

void LCD_writeUint(uint32_t number);

void LCD_writeString(const char *str);

#endif
