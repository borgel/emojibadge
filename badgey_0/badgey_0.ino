
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
  
  (650) 210-6925 
  
 ****************************************************/

#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>

#include "SMS.h"
#include "badgey.h"


Adafruit_8x8matrix g_matrix = Adafruit_8x8matrix();

#define TIME_BETWEEN_WAKE_MS    60000

//for led toggling
int led_toggle_state = LOW;
#define TOGGLE_LED_STATE (led_toggle_state = (led_toggle_state == LOW) ? HIGH : LOW)
    

#define GSM_STR_AT_OK "AT\nOK\n"

//the timestamp in millis when setup finished
unsigned long endOfSetupTimeMs;

void setup() {
  CONSOLE_UART.begin(115200);
  
  g_matrix.begin(0x70);  // pass in the address
  g_matrix.clear();
  
  delay(50);
  
  //setup matrix
  g_matrix.setTextSize(1);
  g_matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  g_matrix.setTextColor(LED_ON);
  
  CONSOLE_UART.println("badgey is alive");
  
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
  
  delay(1000);
  
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
  
  /*
#define CTRL_Z 26
  char buffer1[100];
sprintf (buffer1, "This is my message%c", CTRL_Z);
  GSM_UART.println(buffer1);
  CONSOLE_UART.println(GSM_UART.readString());
  */
  
  
  
  
  
  String resp = gsm_command("AT");
  CONSOLE_UART.println("response was [" + resp + "]");
  if(resp.startsWith("\r\nOK")) {
    CONSOLE_UART.println("GSM radio initialized\n");
  }
  
  //disable echo from the radio
  gsm_command("ATE0");
  
  CONSOLE_UART.println(gsm_command("ATI"));
  delay(100);
  
  
  //TODO ask radio to signal on RI when get SMS (AT+CFGRI=1)
  CONSOLE_UART.println(gsm_command("AT+CFGRI=1"));
  
  //put radio in sms (ascii? text?) mode
  CONSOLE_UART.println(gsm_command("AT+CMGF=1"));
  
  //set message storage for SMS to modem memory (not sim) for read, send, receive
  CONSOLE_UART.println(gsm_command("AT+CPMS=\"ME\",\"ME\",\"ME\""));
  
  //print assorted GSM status info
  
  long bvolt = get_gsm_battery_percent();
  CONSOLE_UART.print("bat volt percent: ");
  CONSOLE_UART.println(bvolt);
  
  //status info
  //carrier name
  CONSOLE_UART.println(gsm_command("AT+COPS?"));
  //print sig strength (9, 10, 11, etc at desk). dont know scale
  CONSOLE_UART.println(gsm_command("AT+CSQ"));
  
  delay(100);
  CONSOLE_UART.println("Setup Done");

  //put somethoing ont he matrix
  g_matrix.clear();
  g_matrix.setTextSize(1);
  g_matrix.setCursor(-1,0);
  g_matrix.print("Hi");
  g_matrix.writeDisplay();
    
  
  CONSOLE_UART.println(gsm_command("AT+CMGL=\"ALL\""));
  
  endOfSetupTimeMs = millis();
  
  //setup ISR to be called on SMS/ring
  attachInterrupt(GSM_RI, isrRing, FALLING);
}



// number of times the device should wake and query for new SMS to display
// wake atleast once on boot to display the last message we got
volatile int rings = 1;

unsigned long lastWakeTime = 0;

void loop() {
    delay(1000);
    
    TOGGLE_LED_STATE;
    digitalWrite(LED_BUILTIN, led_toggle_state);
    
    //TODO get signal strength and display it as a line at MATRIX_MAX_H?
    
    //wake every 30 seconds to display a new SMS if there is one
    if(millis() - lastWakeTime > TIME_BETWEEN_WAKE_MS) {
      rings++;
      
      lastWakeTime = millis() - 1;
      
    }
    
    CONSOLE_UART.println("lastWake = " + String(lastWakeTime) + " millis = " + String(millis()));
    
    //wake up every time we get an alert. often there will be no new SMS to display, but this will refresh it anyway
    if(rings > 0) {
      CONSOLE_UART.println("got a ring");
      
      // consume UART msg that we got a ring (if we did) a single GSM read will do to drain it
      GSM_UART.readString();
      
      SMS_t msg = {0};
      getNextSMS(&msg);
      displayEmoticon(g_matrix, msg.text);
      
      //track that we handled an incoming SMS
      rings--;
      
      //wait TIME_BETWEEN_WAKE_MS before displaying a new emoji
      lastWakeTime = millis() - 1;
      
      //reattach the interrupt
      attachInterrupt(GSM_RI, isrRing, FALLING);
    }
}

/*
  interrupt for tracking radio state
  
  RI is pulled high and goes low for 120ms to indicate ring/SMS
  looks like this works
*/
void isrRing() {
  rings++;
  
  //disable this interrupt until we service the module
  detachInterrupt(GSM_RI);
}

/*
 display the emoticon in the passed string on the matrix
 
 expects emoticons to be 3 chars
 */
void displayEmoticon(Adafruit_8x8matrix matrix, String text){
  matrix.clear();
  
  int len = text.length();
  /*
  if(len == 1) {
  }
  else if
  */
  
  matrix.setCursor(-1,0);
  matrix.print(text[0]);
  
  matrix.setCursor(1,0);
  matrix.print(text[1]);
  
  matrix.setCursor(3,0);
  matrix.print(text[2]);
  
  matrix.writeDisplay();
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
  CONSOLE_UART.println("full bat string: [" + bat + "]");
  //find for '0,'
  bat = bat.substring(bat.indexOf(",") + 1, bat.lastIndexOf(","));
  CONSOLE_UART.println("bat voltage str = [" + bat + "]");
  
  //get the int that's there
  return bat.toInt();
}


  
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
