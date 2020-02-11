#ifndef GENERAL_MACROSES_H
#define GENERAL_MACROSES_H

//====================COMMON DEFS======================
#define HIGH 1
#define LOW  0
#define OK  2
#define NOK 3

#define TRUE  1
#define FALSE 0

#define FOUR_BITS 4
#define FIRST_TETRADE_HIGH 0x0F

//====================COMMON DEFS END======================

#define SET_BIT_TO_LOW(variable, bit)   ((variable) &= ~(HIGH << (bit)))
#define SET_BIT_TO_HIGH(variable, bit)  ((variable) |=  (HIGH << (bit)))

#define BIT_IS_SET_TO_HIGH(variable, bit)  ((variable) &  (HIGH << (bit)))
#define BIT_IS_SET_TO_LOW(variable, bit)  (!((variable) &  (HIGH << (bit))))

typedef enum ResultCodeEnum
{
  RES_LOW  = 0,
  RES_HIGH = 1,
  RES_OK   = 2,
  RES_NOK  = 3,
  RES_PROCESSING_REQUEST    = 4,
  RES_OBJECT_IN_WRONG_STATE = 5,
  RES_UNEXPECTED_INPUT_VAL  = 6,
  RES_UNEXPECTED_PERIP_RESP = 7,
  RES_WRONG_RESP_TIMING     = 8,
  RES_WRONG_DATA_RESP_TIMING= 9
} ResultCode;

#endif
