#include <Wire.h>

#define MATRIX_MAX_H  8
#define MATRIX_MAX_W  8


#define GSM_UART      Serial2
#define CONSOLE_UART  Serial

/* important defines */
//key (to power cycle radio)
#define GSM_KEY       17
#define GSM_PS        16
#define GSM_RI        15


//the rx pin on the GSM breakout. so our tx
//uses UART 2
//const int GSM_RX =    10; //TX2
//const int GSM_TX =    9; //RX2
