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
#define SS   (gpio_num_t)34

SlaveSPI slave(HSPI_HOST);  // VSPI_HOST

const size_t MAX_SIZE = 8;
Array<int,MAX_SIZE> array_current;
Array<int,MAX_SIZE> array_new;

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
    Serial.begin(115200);
    slave.begin(SO, SI, SCLK, SS, 8);
}

void loop() {

    if (slave.getInputStream()->length() && digitalRead(SS) == HIGH) {  // Slave SPI has got data in.
        while (slave.getInputStream()->length()) { //while we have content
            array_new.push_back(slave.readByte()); //lets store it
        }
        
        for(int i=0; i<array_new.size(); i++){
          if(array_current[i] != array_new[i]){ //message has changed, lets print it
            for(int j=0; j<array_new.size(); j++){
              Serial.print("0x");
              Serial.print(array_new[j], HEX);
              Serial.print(" ");
            }
            Serial.println(); //newline
//           Serial << array_new << endl;
           array_current = array_new; //save off for later comparison
           break; //we dont need to continue comparing.
          }
        }
        array_new.clear(); //reset for next round
    }

}
