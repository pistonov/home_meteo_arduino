/* DESCRIPTION
 * This sketch provides an example how to implement a humidity/temperature from Oregon sensor.
 * - Oregon sensor's battery level
 * - Oregon sensor's id
 * - Oregon sensor's type
 * - Oregon sensor's channel
 * - Oregon sensor's temperature
 * - Oregon sensor's humidity
 *
 * MySensors gateway <=======> Arduino UNO <-- (PIN 4)
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
 */

// Enable debug prints
//#define MY_DEBUG

//Define pin where is 433Mhz receiver (AM-RRQ3-443P, pin 4)
//#define MHZ_RECEIVER_PIN 4
//Define maximum Oregon sensors (here, 1 differents sensors)
//#define COUNT_OREGON_SENSORS 1

#include <SPI.h>
#include <EEPROM.h>
#include <Oregon_NR.h>
#include <LedControl.h>
#include <ESP8266WiFi.h>

Oregon_NR oregon(4, 4, 0, true);

const char* ssid = "X";
const char* password = "X";

struct cgr { char c; byte d[5]; };

// 3x5 Numeric Font
static cgr _digits[10] =
{
'0', { 0x07,0x05,0x05,0x05,0x07 },
'1', { 0x02,0x06,0x02,0x02,0x07 },
'2', { 0x07,0x01,0x07,0x04,0x07 },
'3', { 0x07,0x01,0x03,0x01,0x07 },
'4', { 0x05,0x05,0x07,0x01,0x01 },
'5', { 0x07,0x04,0x07,0x01,0x07 },
'6', { 0x07,0x04,0x07,0x05,0x07 },
'7', { 0x07,0x01,0x02,0x02,0x02 },
'8', { 0x07,0x05,0x07,0x05,0x07 },
'9', { 0x07,0x05,0x07,0x01,0x07 },
};

byte err1[] = { 
B00000000,
B00000000,
B00100100,
B00011000,
B00011000,
B00100100,
B00000000,
B00000000 };

//int i=0;
int row=0;
int humid=200;
int tempera=1111;
int tempdiff=1111;
int newtemp=0;
int btnstate=0;
int interval=1800000;
unsigned long previousMillis=0;

// const int BtnPin=5;

LedControl lc=LedControl(13,14,16,1); // Pins: DIN,CLK,CS

void errorshow() { for (int i = 0; i < 7; i++)  { lc.setRow(0,i,err1[i]); }}

void setup(){
//WiFi.begin(ssid, password);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);
  
  lc.shutdown(0,false);
  lc.setIntensity(0,1);
  lc.clearDisplay(0);
#ifdef MY_DEBUG
  Serial.begin(115200);
#endif
  
  //Setup received data
  oregon.start();
  errorshow();
}

void loop(){
    oregon.capture(1); 
    unsigned long currentMillis = millis();
    
    if (currentMillis - previousMillis < 0) {
         previousMillis = 0;
    }
    
    if ((currentMillis - previousMillis) >= interval) {
         lc.clearDisplay(0);
         errorshow();
         previousMillis = currentMillis;
         tempera = 1111;
    }

    if (oregon.captured)  {
      if ((oregon.sens_type == THGN132 || oregon.sens_type == THN132) && oregon.crc_c){

      Serial.print(oregon.sens_tmp, 1);
//      Serial.print(oregon.sens_hmdty, 1);
//      Serial.println(oregon.sens_battery, 1);
 
               newtemp = round(oregon.sens_tmp);
               previousMillis = currentMillis;
               
               if (newtemp >= -50 && newtemp <= 50) {
                 
                   if (tempera == 1111) {
                       tempdiff = 0;
                   }
                   else {
                       tempdiff = tempera - newtemp;
                       tempdiff = abs(tempdiff);
                   }

                   if (tempdiff < 30 && tempera != newtemp) {
                       tempera = newtemp;
                       //humid = round(oregon.sens_hmdty);
                   
                       dispshow(tempera);

                       if (!oregon.sens_battery) {
                           lc.setLed(0,7,7,1);
                       }
                       else {
                           lc.setLed(0,7,7,0);
                       }
                   }
               }
      }
   }
}

void dispshow(int i) {
    lc.clearDisplay(0);
    // Start -- Risuem cifri
    for (int j=0 ; j<5; j++) {
       if (i >= 10 || i <= -10 ) {
           row = (_digits[abs(i) / 10].d[j] << 3) | _digits[abs(i) % 10].d[j];
       }
       else {
           row = _digits[abs(i) % 10].d[j];
       }
       lc.setRow(0,j,row);
    }

    if (i < 0 && i > -10) {
       lc.setLed(0,2,0,1); lc.setLed(0,2,1,1); lc.setLed(0,2,2,1);
    }
    if (i <= -10 && i > -20) {
       lc.setLed(0,2,0,1); lc.setLed(0,2,1,1);
    }
    if (i <= -20) {
       lc.setLed(0,2,0,1);
    }
}

void stopWiFiAndSleep() {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);
}
