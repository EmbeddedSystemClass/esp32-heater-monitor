#define DEBUG false
/*
 *  fahrenheat fsswh2004 SPI monitor
 *
 *  The plan is to put current state on MQTT and make adjustments to the
 *  current setting based on another topic. This will probably work by
 *  establishing the mode to HOLD and then setting the temperature.
 *  Monitoring the LCD buttons will probably help.
 *
 */

#include "vector"

/**
 * SlaveSPI Class
 * - adopt from gist:shaielc/SlaveSPIClass.cpp at https://gist.github.com/shaielc/e0937d68978b03b2544474b641328145
 */
#include "SlaveSPI.h"
#include <SPI.h>

SlaveSPI spi1(VSPI_HOST);
SlaveSPI spi2(HSPI_HOST);

const size_t MAX_SIZE = 8;
std::vector<int> spi_data;
std::vector<int> inbound_spi_buffer;

bool isChanged = false;

/* 2 part LCD, two SPI slaves. We will consume both to establish the screen's state.
 *
 *  LCD 5V -> 3.3v via TI chip
 * Pin 1 = CS -   GPIO35    SS1 (top of LCD)
 * Pin 2 = CLK -  GPIO27
 * Pin 3 = MOSI - GPIO25
 * Pin 4 = CS - not used
 * Pin 5 = CS -   GPIO34    SS2 (bottom of LCD)
 * Pin 6 = Ground
 * Pin 7 = 5V
 */

#define SO   (gpio_num_t)32 //not used. should look into avoiding it alltogether.
#define SI   (gpio_num_t)25
#define SCLK (gpio_num_t)27
#define SS1  (gpio_num_t)35
#define SS2  (gpio_num_t)34

/* Buttons (pull low)
 * Pin 1 = power
 * Pin 2 = down
 * Pin 3 = up
 * Pin 4 = mode
 */

#define POWER_PIN 5
#define DOWN_PIN 17
#define UP_PIN 16
#define MODE_PIN 4
#define BUTTON_DELAY 500

/*
 * Introducing some state
*/

int mainDisplayValue = -1;

void press_button(int pin){
  if(DEBUG){
    Serial.print("Pressing pin: ");
    Serial.println(pin);
  }

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(500);
  pinMode(pin, INPUT);
}

void set_hold_mode(){
    press_button(POWER_PIN);
    delay(BUTTON_DELAY);
    press_button(POWER_PIN);
    delay(BUTTON_DELAY);
    press_button(MODE_PIN);
    delay(BUTTON_DELAY);
    press_button(MODE_PIN);
    delay(BUTTON_DELAY);
    press_button(MODE_PIN);
}

void setup() {
  Serial.begin(115200);
  spi1.begin(SO, SI, SCLK, SS1, 8);
  spi2.begin(SO, SI, SCLK, SS2, 8);
  pinMode(POWER_PIN, INPUT);
  pinMode(DOWN_PIN, INPUT);
  pinMode(UP_PIN, INPUT);
  pinMode(MODE_PIN, INPUT);
  delay(2000); //let the heater stabilize
  set_hold_mode();
}

void printBinPadded(int nb){
  int   i = 7;

  while (i >= 0) {
    if ((nb >> i) & 1)
      Serial.print("1");
    else
      Serial.print("0");
    --i;
  }
}

void getSpiContent(){
  if (spi1.getInputStream()->size()) {  // spi1 SPI has got data in.
    while (spi1.getInputStream()->size() && inbound_spi_buffer.size() < 8) { //while we have content
        inbound_spi_buffer.push_back(spi1.readByte()); //lets store it
    }
    if(inbound_spi_buffer.size()>8){
      return;
    }
    spi1.flushInputStream(); //idk what is left... but its junk.
    while (digitalRead(SS2) == LOW){
      //wait for the second half of the message
    }
    while (inbound_spi_buffer.size() < 16) { //while we have content
      if(spi2.getInputStream()->size()){
          inbound_spi_buffer.push_back(spi2.readByte()); //lets store it
      }
    }
    spi2.flushInputStream();

    if(spi_data != inbound_spi_buffer && inbound_spi_buffer.size() == 16){ //total packet size is 16 bytes
      isChanged=true;   //set flag so change is processed
      spi_data = inbound_spi_buffer; //save off for later use
    }
    inbound_spi_buffer.clear(); //reset for next round
  }
}

void printSPIData(){
  if(isChanged){
    // Serial.print(millis());
    // Serial.print(" ");
    for(int a = 0; a<spi_data.size(); a++){
      Serial.print("0x");
      Serial.printf("%02x",spi_data[a]);
      Serial.print(" ");
      // Serial.print("0b");
      // printBinPadded(spi_data[a]);
      // Serial.print(" ");
    }
    // Serial.print("\n");
  }
}

int lcdDigitToInt(int digit){
  int realValue = -1;
  switch(digit){ //&0xfe we only care about bits 2-8. bit 1 is HOLD on index 4
    case 0x30: //1
      realValue = 1;
      break;
    case 0xae: //2
      realValue = 2;
      break;
    case 0xb6: //3
      realValue = 3;
      break;
    case 0x74: //4
      realValue = 4;
      break;
    case 0xd6: //5
      realValue = 5;
      break;
    case 0xde: //6
      realValue = 6;
      break;
    case 0xb0: //7
      realValue = 7;
      break;
    case 0xfe: //8
      realValue = 8;
      break;
    case 0xf6: //9
      realValue = 9;
      break;
    case 0x00:
    case 0xfa: //0
      realValue = 0;
      break;
  }
  if(DEBUG){
  Serial.print("Parsed value: ");
  Serial.print(realValue);
  Serial.print(" ");
  }
  return realValue;
}

void parseLCD(){
  if(isChanged){
    const int idleMessage[16] = {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    bool idle = false;
    if(spi_data.data() == idleMessage){
      idle = true;
    }

    int rawDigit1 = spi_data[12]&0xfe;
    int rawDigit2 = spi_data[14];
    int holdMode = spi_data[12]&0x1;

    int mainDigits = 10*lcdDigitToInt(rawDigit1);
    mainDigits+=lcdDigitToInt(rawDigit2);

    if (mainDigits>=0){ // negative values are not valid. Controller seems to have some non-digit values on the SPI bus in the bits usually for digits when idle
      if(mainDigits != mainDisplayValue){
        mainDisplayValue = mainDigits;
      }
    }
    Serial.print("Idle: ");
    Serial.print(idle);
    Serial.print(" Hold: ");
    Serial.print(holdMode);
    Serial.print(" Main digits: ");
    Serial.print(mainDisplayValue);
    Serial.print(" ");
  }
}

int incomingByte = 0;   // for incoming serial data

void loop() {
  getSpiContent();

  if (isChanged){
    parseLCD();
    printSPIData();
    Serial.println();
    isChanged = false;
  }

  //will be used for testing purposes
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();

    // say what you got:
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
  }
}
