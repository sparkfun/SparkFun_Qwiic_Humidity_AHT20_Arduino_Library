/****************************************************************************
 * Tests the begin and isConnected function of the Qwiic Humidity library
 * 
 * Priyanka Makin @ SparkFun Electronics
 * Original Creation Date: March 26, 2020
 * 
 * SparkFun labored with love to create this code. Feel like supporting open
 * source hardware? Buy a board from SparkFun! LINK GOES HERE
 * 
 * This code is lemonadeware; if you see me (or any other SparkFun employee)
 * at the local, and you've found our code helpful, please buy us a round!
 * 
 * Hardware Connections:
 * Attach RedBoard to computer using micro-B USB cable.
 * Attach Qwiic Humidity board to RedBoard using Qwiic cable.
 * 
 * Distributed as-is; no warranty is given.
 ****************************************************************************/
#include <Wire.h>

#include <SparkFun_Qwiic_Humidity_AHT20.h>  //Click here to get the library: LINK GOES HERE
AHT20 humidity;

void setup()
{
  Serial.begin(115200);
  Serial.println("Qwiic Humidity AHT20 examples");
  Wire.begin(); //Join I2C bus

  //Check if the sensor will acknowledge
  if (humidity.begin() == false)
  {
    Serial.println("Device did not acknowledge! Freezing.");
    while(1);
  }
  Serial.println("AHT20 acknowledged.");
}

void loop()
{
}
