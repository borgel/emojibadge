
/*************************************************** 
  This is a library for our I2C LED Backpacks

  Designed specifically to work with the Adafruit LED Matrix backpacks 
  ----> http://www.adafruit.com/products/872
  ----> http://www.adafruit.com/products/871
  ----> http://www.adafruit.com/products/870

  These displays use I2C to communicate, 2 pins are required to 
  interface. There are multiple selectable I2C addresses. For backpacks
  with 2 Address Select pins: 0x70, 0x71, 0x72 or 0x73. For backpacks
  with 3 Address Select pins: 0x70 thru 0x77
  
  GSM board: https://learn.adafruit.com/adafruit-fona-mini-gsm-gprs-cellular-phone-module/pinouts
  
 ****************************************************/

#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>

Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

//for led toggling
int led_toggle_state = LOW;
#define TOGGLE_LED_STATE (led_toggle_state = (led_toggle_state == LOW) ? HIGH : LOW)

/* important defines */
//key (to power cycle radio)
const int GSM_KEY =   17;
const int GSM_PS =    16;
const int GSM_RI =    15;

//the rx pin on the GSM breakout. so our tx
//uses UART 2
//const int GSM_RX =    10; //TX2
//const int GSM_TX =    9; //RX2
#define GSM_UART      Serial2

#define GSM_STR_AT_OK    "AT\nOK\n"

#define CONSOLE_UART  Serial


void setup() {
  CONSOLE_UART.begin(115200);
  CONSOLE_UART.println("badgey is alive");
  
  matrix.begin(0x70);  // pass in the address
  matrix.clear();
  
  //GPIO
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(GSM_KEY, OUTPUT);
  pinMode(GSM_PS, INPUT);
  pinMode(GSM_RI, INPUT);
  
  digitalWrite(LED_BUILTIN, HIGH);
  
  //setup uart
  GSM_UART.begin(9600);
  
  /* init GSM module */
  //pull key pin low
  digitalWrite(GSM_KEY, LOW);
  
  //wait until the power indicator goes low (~2 seconds)
  while(digitalRead(GSM_PS) == LOW) {
    //ms
    delay(500);
    
    TOGGLE_LED_STATE;
    digitalWrite(LED_BUILTIN, led_toggle_state);
  }
  // stop keying the radio
  digitalWrite(GSM_KEY, HIGH);
  
  CONSOLE_UART.println("GSM radio on\n");
  
  digitalWrite(LED_BUILTIN, LOW);
  
  //sync the gsm's uart (it autodetects baud)
  gsm_command("AT");
  
  delay(100);
  
  String resp = gsm_command("AT");
  CONSOLE_UART.println("response was [" + resp + "]");
  if(resp == GSM_STR_AT_OK) {
    CONSOLE_UART.println("GSM radio initialized\n");
  }
  
  //disable echo from the radio
  gsm_command("ATE0");
  
  CONSOLE_UART.println(gsm_command("ATI"));
  delay(200);
  CONSOLE_UART.println(gsm_command("AT+CBC"));

}

/*
  Send a command to the GSM radio and return a String of the response
*/
String gsm_command(String com) {
  //using println does \n for us
  GSM_UART.println(com);
  return GSM_UART.readString();
}

/*
  TODO
  query the GSM breakout for the charge of the battery (in %)
  returns: integer percentage from 100 to 0
*/
int get_gsm_battery_percent(){
  String bat = gsm_command("AT+CBC\n");
  
  //reply is:
  /*
  +CBC: 0,93,3947

  OK
  */
  //split on commas. take int between 1st and 2nd
  return 0;
}

void loop() {
  matrix.setTextSize(1);
  matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix.setTextColor(LED_ON);
  
    matrix.clear();
    matrix.setCursor(-1,1);
    matrix.print(":");
    matrix.setCursor(3,1);
    matrix.print(")");
    matrix.writeDisplay();

    delay(100);
    
    TOGGLE_LED_STATE;
    digitalWrite(LED_BUILTIN, led_toggle_state);
  
  /*
  for (int8_t x=0; x>=-36; x--) {
    matrix.clear();
    matrix.setCursor(x,0);
    matrix.print("Hello World");
    matrix.writeDisplay();
    delay(100);
  }
  
  
  matrix.setRotation(3);
  for (int8_t x=7; x>=-36; x--) {
    matrix.clear();
    matrix.setCursor(x,0);
    matrix.print("World");
    matrix.writeDisplay();
    delay(100);
  }
  matrix.setRotation(0);
  */
}
