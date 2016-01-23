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
LedControl lc=LedControl(2,1); // Load pin, number of LED displays

/* we always wait a bit between updates of the display */
unsigned long delaytime=100;

void setup() {
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);
}

/*
 This method will display the characters for the
 word "Arduino" one after the other on the matrix. 
 (you need at least 5x7 leds to see the whole chars)
 */
void writeArduinoOnMatrix() {
  /* here is the data for the characters 
  byte a[5]={B01111110,B10001000,B10001000,B10001000,B01111110};
  byte r[5]={B00111110,B00010000,B00100000,B00100000,B00010000};
  byte d[5]={B00011100,B00100010,B00100010,B00010010,B11111110};
  byte u[5]={B00111100,B00000010,B00000010,B00000100,B00111110};
  byte i[5]={B00000000,B00100010,B10111110,B00000010,B00000000};
  byte n[5]={B00111110,B00010000,B00100000,B00100000,B00011110};
  byte o[5]={B00011100,B00100010,B00100010,B00100010,B00011100};*/

  /* here is the data for the characters */  
  byte a[8]={0x04, 0x0A, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x00}; // A
  byte r[8]={0x00, 0x00, 0x0D, 0x13, 0x01, 0x01, 0x01, 0x00}; // r
  byte d[8]={0x10, 0x10, 0x16, 0x19, 0x11, 0x19, 0x16, 0x00}; // d
  byte u[8]={0x00, 0x00, 0x11, 0x11, 0x11, 0x19, 0x16, 0x00}; // u
  byte i[8]={0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00}; // i
  byte n[8]={0x00, 0x00, 0x0D, 0x13, 0x11, 0x11, 0x11, 0x00}; // n
  byte o[8]={0x00, 0x00, 0x06, 0x09, 0x09, 0x09, 0x06, 0x00}; // o

 byte een[6]={0x00, 0x3e, 0x51, 0x49, 0x45, 0x3e};
 byte twee[6]={0x00, 0x42, 0x61, 0x51, 0x49, 0x46};
 byte drie[6]={0x00, 0x21, 0x41, 0x45, 0x4b, 0x31};
 byte vier[6]={0x00, 0x18, 0x14, 0x12, 0x7f, 0x10};
 byte vijf[6]={0x00, 0x27, 0x45, 0x45, 0x45, 0x39};
 byte zes[6]={0x00, 0x3c, 0x4a, 0x49, 0x49, 0x30};
 byte zeven[6]={0x00, 0x01, 0x71, 0x09, 0x05, 0x03};
 byte acht[6]={0x00, 0x36, 0x49, 0x49, 0x49, 0x36};
 byte negen[6]={0x00, 0x00, 0x36, 0x36, 0x00, 0x00};


/*   "1" [0x00 0x00 0x42 0x7f 0x40 0x00]
   "2" [0x00 0x42 0x61 0x51 0x49 0x46]
   "3" [0x00 0x21 0x41 0x45 0x4b 0x31]
   "4" [0x00 0x18 0x14 0x12 0x7f 0x10]
   "5" [0x00 0x27 0x45 0x45 0x45 0x39]
   "6" [0x00 0x3c 0x4a 0x49 0x49 0x30]
   "7" [0x00 0x01 0x71 0x09 0x05 0x03]
   "8" [0x00 0x36 0x49 0x49 0x49 0x36]
   "9" [0x00 0x06 0x49 0x49 0x29 0x1e]
   ":" [0x00 0x00 0x36 0x36 0x00 0x00]

*/


  
  
  /* now display them one by one with a small delay */
  lc.setRow(0,7,a[0]);
  lc.setRow(0,6,a[1]);
  lc.setRow(0,5,a[2]);
  lc.setRow(0,4,a[3]);
  lc.setRow(0,3,a[4]);
  lc.setRow(0,2,a[5]);
  lc.setRow(0,1,a[6]);
  lc.setRow(0,0,a[7]);
}

/*
  This function lights up a some Leds in a row.
 The pattern will be repeated on every row.
 The pattern will blink along with the row-number.
 row number 4 (index==3) will blink 4 times etc.
 */
void rows() {
  for(int row=0;row<8;row++) {
    delay(delaytime);
    lc.setRow(0,row,B10100000);
    delay(delaytime);
    lc.setRow(0,row,(byte)0);
    for(int i=0;i<row;i++) {
      delay(delaytime);
      lc.setRow(0,row,B10100000);
      delay(delaytime);
      lc.setRow(0,row,(byte)0);
    }
  }
}

/*
  This function lights up a some Leds in a column.
 The pattern will be repeated on every column.
 The pattern will blink along with the column-number.
 column number 4 (index==3) will blink 4 times etc.
 */
void columns() {
  for(int col=0;col<8;col++) {
    delay(delaytime);
    lc.setColumn(0,col,B10100000);
    delay(delaytime);
    lc.setColumn(0,col,(byte)0);
    for(int i=0;i<col;i++) {
      delay(delaytime);
      lc.setColumn(0,col,B10100000);
      delay(delaytime);
      lc.setColumn(0,col,(byte)0);
    }
  }
}

/* 
 This function will light up every Led on the matrix.
 The led will blink along with the row-number.
 row number 4 (index==3) will blink 4 times etc.
 */
void single() {
  for(int row=0;row<8;row++) {
    for(int col=0;col<8;col++) {
      delay(delaytime);
      lc.setLed(0,row,col,true);
      delay(delaytime);
      for(int i=0;i<col;i++) {
        lc.setLed(0,row,col,false);
        delay(delaytime);
        lc.setLed(0,row,col,true);
        delay(delaytime);
      }
    }
  }
}

void loop() { 
  writeArduinoOnMatrix();
//  rows();
//  columns();
//  single();
}
