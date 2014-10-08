#include "badgey.h"
#include "SMS.h"

#define    AT_SMS_STORAGE          "AT+CPMS=\"ME\",\"ME\",\"ME\""

//sms strings always have 3 cl/lf combos and an 'OK' at end
#define   TEXT_END_STRING_OFFSET   8

void getSMS(String index, SMS_t *msg){
  /* this is a response
get msg ln 0: [
+CMGR: "REC READ","14087611826","","14/10/02,23:49:38-28"
Free Msg: Unable to send message - Message Blocking is active.

OK
*/


  if(!msg || index.length() == 0){
    return;
  }
  
  //AT+CMGR=123 where 123 is the msg number
  GSM_UART.println("AT+CMGR=" + index);
  
  delay(50);
  
  //gets ENTIRE message with all metadata
  String resp = GSM_UART.readString();
  
  CONSOLE_UART.println("get msg line 0: [" + resp);
  
  /*
  for(int i = 0; i < resp.length(); i++) {
    CONSOLE_UART.println(String((int)resp[i]) + "\t[" + resp[i] + "]");
  }
  */
  
  //get to after first quote
  int before = resp.indexOf(',') + 2;
  //find second quote
  int after = resp.indexOf('"', before + 1);
  
  String fromNumber = resp.substring(before, after);
  
  CONSOLE_UART.println("sms sent from: [" + fromNumber + "]");
  
  //split on cl/lf. but after the one at position 0
  before = resp.indexOf((char)10, after); //ascii 10?
  
  //get the rest of the message from after the first line
  String messageText = resp.substring(before + 1, resp.length() - TEXT_END_STRING_OFFSET);
  
  CONSOLE_UART.println("lf at " + String(before) + " message text was [" + messageText + "]");

  //copy to destination
  //FIXME TODO fill in msgIndex?
  fromNumber.toCharArray(msg->from, PHONE_NUMBER_MAX_LEN - 1);
  messageText.toCharArray(msg->text, MAX_SMS_LEN - 1);
}

/*
Get the first message from the listing of all messages.
Note, this assumes there is at least 1 total message!
*/
void getFirstSMS(SMS_t *msg) {
  //print all sms
  //parse first one
  
  GSM_UART.println("AT+CMGL=\"ALL\"");
  delay(50);
  String allmsgs = GSM_UART.readString();
  
  //find index
  int before = allmsgs.indexOf(':') + 1;
  int after = allmsgs.indexOf(',' , before + 1);
  String val = allmsgs.substring(before, after).trim();
  val.toCharArray(msg->msgIndex, MSG_INDEX_LEN - 1);
  
  CONSOLE_UART.println("first sms is index [" + val + "]");
  
  //trash the rest of the response and do a single get message. makes it easier to find the boundries between sms's
  getSMS(val, msg);
  
  /*
  //get to after first quote
  before = val.indexOf(',') + 2;
  //find second quote
  after = val.indexOf('"', before + 1);
  
  val = val.substring(before, after);
  val.toCharArray(msg->from, PHONE_NUMBER_MAX_LEN - 1);
  
  //split on cl/lf. but after the one at position 0
  before = val.indexOf((char)10, after); //ascii 10?
  
  //get the rest of the message from after the first line
  //FIXME make this read until end of msg
  //how do we know that?
  
  val = val.substring(before + 1, resp.length() - TEXT_END_STRING_OFFSET);
  
  val.toCharArray(msg->text, MAX_SMS_LEN - 1);
  */
}


//done
void getNextSMS(SMS_t *msg){
  int total = totalSMS();
  
  CONSOLE_UART.println("total = " + String(total));
  
  //FIXME remove hard code. figure out how to do this
  getFirstSMS(msg);
  
  // delete the message we just got if there are more then 1 remaining
  if(total > 1) {
    CONSOLE_UART.println("was more then 1 message. deleting the one we got [" + String(msg->msgIndex) + "]");
    CONSOLE_UART.println(deleteSMS(msg->msgIndex));
  }
}


//works
int totalSMS(){
  //sets message storage. replies with totals IE [\n+CPMS: 8,80,8,50,8,50\n\nOK\n]
  //CONSOLE_UART.println(gsm_command("AT+CPMS=\"MT\",\"ME\",\"ME\""));
  
  GSM_UART.println(AT_SMS_STORAGE);
  String resp = GSM_UART.readString();
  
  int colon = resp.indexOf(':');
  resp = resp.substring(colon + 1, resp.indexOf(',', colon));
  
  return (int)resp.toInt();
}

//works? untested but complete
String deleteSMS(String index){
  GSM_UART.println("AT+CMGD=" + String(index));
  return GSM_UART.readString();
}

void sendSMS(int number, String text) {  
  /*
  CONSOLE_UART.println("send sms:" + gsm_command("AT+CMGS=\"14087611826\""));

#define CTRL_Z 26
  char buffer[100];
sprintf (buffer, "This is my message%c", CTRL_Z);

  GSM_UART.println(buffer);
  CONSOLE_UART.println(GSM_UART.readString());
  CONSOLE_UART.println(GSM_UART.readString());
  */
}
