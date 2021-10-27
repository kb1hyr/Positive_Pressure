/**
* This is a positive pressure system for maintaining a cleanroom, laminar flow box or similar.
* The master unit senses pressure inside the space, and the slave unit senses pressure from 
* outside the space, and sends that information wirelessly over Xbee.
* The master unit uses the differential in pressure to control the control voltage
* for an industrial climate control fan that is blowing filtered air into the space.

* This code is for the master device.  

* Hardware:
* Arduino Uno R3 or Equivalent
* XBee
* Seeedstudio Xbee Shield configured for pins 4 & 5
* Sparkfun Serial LCD attached to pins 2&3
* Sparkfun MCP4725 DAC attached via I2C 
* The DAC is amplified via an LM358 op amp.
* Can use either BMP180 or BMP280 for pressure sensing depending on conditionals
*
* @author Brett Markham
* @version 1.2
* @license LGPL
* @copyright Brett Markham 2017
*/


// ET_DRIFT is set experimentally.  The number is changed until the two units sitting 
// beside each other show the same pressure
#define ET_DRIFT 3
#define BMP180
//#define BMP280



#include <Wire.h>
#ifdef BMP180
#include <Adafruit_BMP085.h>
#endif
#ifdef BMP280
#include "cactus_io_BME280_I2C.h"
#endif
#include <Adafruit_MCP4725.h> // The DAC
#include <SoftwareSerial.h>
#include "everytime.h"
#include <avr/sleep.h>
#include <ResponsiveAnalogRead.h>

void clearDisplay();

SoftwareSerial lcd(2, 3);
SoftwareSerial xbee(4, 5);  //RX is pin 4, TX is Pin 5
#ifdef BMP180
Adafruit_BMP085 pressure;
#endif
#ifdef BMP280
BME280_I2C pressure; // using I2C 0x77 to use 0x76, do BME280_I2C pressure(0x76)
#endif
Adafruit_MCP4725 dac;
ResponsiveAnalogRead analog(0, true);
int SWITCH = 7; // on/off switch
long RPmb, Dmb;
long APmb;
char junk;
int SwitchOn = 0;
int VoltageFactor = 256; //steps per output volt of DAC
int step = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(9600);
  xbee.begin(9600);
  pinMode(SWITCH, INPUT);
  clearDisplay();
  Serial.println("Hello!");

  pressure.begin();

  // For Adafruit MCP4725A1 the address is 0x62 (default) or 0x63 (ADDR pin tied to VCC)
  // For MCP4725A0 the address is 0x60 or 0x61
  // For MCP4725A2 the address is 0x64 or 0x65
  dac.begin(0x60); // Sparkfun unit
}

void GetSwitchState()
{
  if (digitalRead(SWITCH) == HIGH)
          {
            SwitchOn = 1;
          }
          else
          {
             SwitchOn = 0;
          }  
}

void GetXbeeData()
{
  RPmb = -1;
  while (RPmb < 0) {
  	while (xbee.available()==0); //wait until data comes
  	RPmb=(long)xbee.parseInt(); // get the pressure
  	while (xbee.available()>0)
  		{
    		junk = xbee.read(); // 
  		}
  	RPmb=RPmb-24; // fudge  factor for differences between sensors
  }
}


void GetLocalPressure()
{
  long reading;
  #ifdef BMP180
  reading = pressure.readPressure();
  #endif
  #ifdef BMP280
  float P;
  pressure.readSensor();
  P = getPressure_MB(); // in millibars
  reading = (longint)P*100; // convert mbar to Pa
  #endif
  analog.update(reading);
  APmb = analog.getValue();
}

void WriteToLCD ()
{
   lcd.write(0xFE);  // send the special command
   lcd.write(0x01);  // send the clear screen command
   // Print out the measurement:
   lcd.print("RP: ");
   lcd.print(RPmb);
  
    if (SwitchOn == 1)
    {
      lcd.print(" ON");
    }
    else
    {
      lcd.print(" OFF");
    }
    lcd.write(0x0A);  // send the linefeed command
    lcd.print("LP: ");
    lcd.print(APmb);
    lcd.print(" ");
    lcd.print(step);
    lcd.println("V");
}

void SetVoltage()
{
Dmb = APmb - RPmb; // Pressure difference = inside - outside
  if (SwitchOn == 1)
  {
    if (Dmb < 25) 
      {
        step = step + 1;
        delay(1000);
      }
    if ((Dmb >= 25) && (Dmb < 50)) step = step;
    if (Dmb >= 50) step = step - 1;
    if (step <= 0) step = 1;
    if (step > 10) step = 10;
  }
  else  // if switch is off 
  {
    step = 0;
  }
  dac.setVoltage(step*VoltageFactor, false);  
}

void loop() {
  GetSwitchState();  
  GetLocalPressure(); 
  GetXbeeData();
  every(10000){
  SetVoltage();
  }
  WriteToLCD();
}



void clearDisplay()
{
  lcd.write(0xFE);  // send the special command
  lcd.write(0x01);  // send the clear screen command
}
