//We always have to include the library
#include <SPI.h>
#include "LedControlSPIESP8266.h"

/* needs at least a 8X8 dot matrix! Up to 8 can be cascaded*/

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 Pinout sur NodeMCU 
 D5 = GPIO14 = Clk   is connected to CLK  [pin 13 on max7219]
 D7 = GPIO13 = MOSI  is connected to DATA in [pin 1 on max7219]
 D8 = GPIO15 = SS    is connected to LOAD [pin 12 on max7219]
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(15,1); // Load pin, number of LED displays

/* we always wait a bit between updates of the display */
unsigned long delaytime=10;

/* 
 This time we have more than one device. 
 But all of them have to be initialized 
 individually.
 */
void setup() {
  //we have already set the number of devices when we created the LedControl
  int devices=lc.getDeviceCount();
  //we have to init all devices in a loop
  for(int address=0;address<devices;address++) {
    /*The MAX72XX is in power-saving mode on startup*/
    lc.shutdown(address,false);
    /* Set the brightness to a medium values */
    lc.setIntensity(address,8);
    /* and clear the display */
    lc.clearDisplay(address);
  }
}

void loop() { 
  //read the number cascaded devices
  int devices=lc.getDeviceCount();
  
  //we have to init all devices in a loop
  for(int row=0;row<8;row++) {
    for(int address=0;address<devices;address++) {
      for(int col=0;col<8;col++) {
        //delay(delaytime);
        lc.setLed(address,row,col,true);
        delay(delaytime);
        lc.setLed(address,row,col,false);
      }
    }
  }
}