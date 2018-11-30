#include <Array.h> //https://github.com/janelia-arduino/Array/ so I font have to ;)

/**
 * SlaveSPI Class
 * - adopt from gist:shaielc/SlaveSPIClass.cpp at https://gist.github.com/shaielc/e0937d68978b03b2544474b641328145
 */
#include "SlaveSPI.h"
#include <SPI.h>

#define SO   (gpio_num_t)32 //not used. should look into avoiding it alltogether.
#define SI   (gpio_num_t)25
#define SCLK (gpio_num_t)27
#define SS1  (gpio_num_t)35
#define SS2  (gpio_num_t)34

SlaveSPI spi1(VSPI_HOST); 
SlaveSPI spi2(HSPI_HOST);  

const size_t MAX_SIZE = 8;
Array<int,MAX_SIZE> spi1_array_current;
Array<int,MAX_SIZE> spi1_array_new;
Array<int,MAX_SIZE> spi2_array_current;
Array<int,MAX_SIZE> spi2_array_new;

bool isChanged = false;

/* LCD 5V -> 3.3v via TI chip
 * Pin 1 = CS - not used
 * Pin 2 = CLK - GPIO27
 * Pin 3 = MOSI - GPIO25
 * Pin 4 = CS - not used
 * Pin 5 = CS - GPIO34
 */

/* Buttons (pull low) 
 * Pin 1 = power
 * Pin 2 = down
 * Pin 3 = up
 * Pin 4 = mode
 */


void setup() {
  Serial.setDebugOutput(true);
    Serial.begin(115200);
    spi1.begin(SO, SI, SCLK, SS1, 8);
    spi2.begin(SO, SI, SCLK, SS2, 8);
    
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

void loop() {
    if (spi1.getInputStream()->length() && digitalRead(SS1) == HIGH) {  // spi2 SPI has got data in.
        
        while (spi1.getInputStream()->length()) { //while we have content
            spi1_array_new.push_back(spi1.readByte()); //lets store it
        }
        if(spi1_array_new.size() < 8){ //message is too short, lets start over
            spi1_array_new = spi1_array_current;
          }
        
        for(int i=0; i<spi1_array_new.size(); i++){
          if(spi1_array_current[i] != spi1_array_new[i]){ 
            isChanged=true;   //set flag so change is printed
            spi1_array_current = spi1_array_new; //save off for later use
            break; //we dont need to continue comparing.
            }
          }
        spi1_array_new.clear(); //reset for next round
    
      while (digitalRead(SS2) == LOW){
        //wait for the second half of the message
      }
      if (spi2.getInputStream()->length() && digitalRead(SS2) == HIGH) {  // spi2 SPI has got data in.
          
          while (spi2.getInputStream()->length()) { //while we have content
              spi2_array_new.push_back(spi2.readByte()); //lets store it
          }
          if(spi2_array_new.size() < 8){ //message is too short, lets start over
            spi2_array_new = spi2_array_current;
            }
          for(int k=0; k<spi2_array_new.size(); k++){
            if(spi2_array_current[k] != spi2_array_new[k]){ //message has changed, lets print it
              isChanged=true;            
              spi2_array_current = spi2_array_new; //save off for later comparison
              break; //we dont need to continue comparing.
              }
           }
          spi2_array_new.clear(); //reset for next round
      }
    }
    if(isChanged){
      Serial.print(millis());
      Serial.print(" ");
      for(int a = 0; a<spi1_array_current.size(); a++){
        Serial.print("0x");
        Serial.printf("%02x",spi1_array_current[a]);
        Serial.print(" ");
        
        Serial.print("0b");
        printBinPadded(spi1_array_current[a]);
        Serial.print(" ");
      }
      for(int b = 0; b<spi2_array_current.size(); b++){
        Serial.print("0x");
        Serial.printf("%02x",spi2_array_current[b]);
        Serial.print(" ");
        
        Serial.print("0b");
        printBinPadded(spi2_array_current[b]);
        Serial.print(" ");
      }
      Serial.print("\n");
      isChanged=false;
    }

}
