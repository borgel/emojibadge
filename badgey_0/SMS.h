/*
 API for managing messages
 */

#include <Wire.h>

//140 char + 1 for terminator
#define    MAX_SMS_LEN             141
//15 char len + terminator
#define    PHONE_NUMBER_MAX_LEN    16
//3 char len + terminator
#define    MSG_INDEX_LEN           4

struct SMS_t {
  char msgIndex[MSG_INDEX_LEN];
  char from[PHONE_NUMBER_MAX_LEN];
  char text[MAX_SMS_LEN];
};

/*
get the total number of avalible SMS'
*/
int totalSMS();

/*
delete the SMS at the given index
*/
String deleteSMS(String index);

/*
Get the SMS on the device at the provided index
*/
void getSMS(String index, SMS_t *msg);

/*
get the next SMS to display
If there are more after this one, then this deletes the returned message.
If this is the last SMS, then it is left on the device
*/
void getNextSMS(SMS_t *msg);

