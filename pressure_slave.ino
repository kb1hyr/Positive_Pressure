/**
* This is a positive pressure system for maintaining a cleanroom, laminar flow box or similar.
* The master unit senses pressure inside the space, and the slave unit senses pressure from 
* outside the space, and sends that information wirelessly over Xbee.
* The master unit uses the differential in pressure to control the control voltage
* for an industrial climate control fan that is blowing filtered air into the space.

* This code is for the slave device.  

* Hardware:
* Arduino Uno R3 or equivalent
* Seeedstudio Xbee Shield configured w/ jumpers for RX pin 4 and TX pin 5
* Digi Xbee Series 1
* Sparkfun BMP180 breakout board 
* 
* Once every second this device reads the raw barometric pressure (in Pascals)
* It then sends it via Xbee to the master device. 
*
* @author Brett Markham
* @version 1.2
* @license LGPL
* @copyright Brett Markham 2017
*/ 

/******* Define the sensor this is for.  Choose ONE of the below and comment out the other ********/
//#define BMP280
#define BMP180
/*************************************************************************************************/


/***************  Includes **********************************************************************/
#include <Wire.h>
#include <SoftwareSerial.h>
#ifdef BMP180
#include <Adafruit_BMP085.h> // also works for BMP180
#endif 

#ifdef BMP280
#include "cactus_io_BME280_I2C.h"
#endif
#include <ResponsiveAnalogRead.h>
/***********************************************************************************************/

/****************** Miscellaneous Defines *******************************************************/

/***********************************************************************************************/

/***************************** Global variables *************************************************/
long RPmb; // Pressure in pascals is usually in the 97000 to 120000 range so longint is needed
#ifdef BMP180
Adafruit_BMP085 pressure;
#endif
#ifdef BMP280
BME280_I2C pressure; // using I2C 0x77 to use 0x76, do BME280_I2C pressure(0x76)
#endif
ResponsiveAnalogRead analog(0, true);
long reading;
SoftwareSerial xbee(4, 5);  //RX is pin 4, TX is Pin 5
/*************************************************************************************************/



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  xbee.begin(9600);
  Serial.println("Hello!");
  pressure.begin();
  }

void loop() {
  #ifdef BMP180	
  reading = pressure.readPressure(); // in Pascals
  #endif
  #ifdef BMP280
  float P;
  pressure.readSensor();
  P = getPressure_MB(); // in millibars
  reading = (longint)P*100; // convert mbar to Pa
  #endif
  analog.update(reading);
  RPmb = analog.getValue();
  
  xbee.println(RPmb);
  delay(1000);  // Pause for 1 second.  
}
