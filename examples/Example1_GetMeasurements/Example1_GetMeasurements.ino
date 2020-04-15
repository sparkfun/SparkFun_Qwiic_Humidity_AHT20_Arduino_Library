/**************************************************************************
 * Tests the getTemperature and getHumidity functions of the Qwiic Humidity
 * library
 * 
 * Priyanka Makin @ SparkFun Electronics
 * Original Creation Date: March 31, 2020
 * 
 * SparkFun labored with love to create this code. Feel like supporting open
 * source hardware? Buy a board from SparkFun! LINK GOES HERE
 * 
 * This code is lemonadeware; if you see me (or any other SparkFun employee)
 * at the local, and you've found our code helpful, please buy us a round!
 * 
 * Hardware Connections:
 * Attach a RedBoard to computer using micro-B USB cable.
 * Attach a Qwiic Humidity board to RedBoard using Qwiic cable.
 * 
 * Distributed as-is; no warranty is given.
 **************************************************************************/
#include <Wire.h>

#include <SparkFun_Qwiic_Humidity_AHT20.h>  //Click here to get the library: LINK GOES HERE
AHT20 humiditySensor;

float temperature;
float humidity;

void setup()
{
  Serial.begin(115200);
  Serial.println("Qwiic Humidity AHT20 examples");
  Wire.begin(); //Join I2C bus

  //Check if the sensor will acknowledge
  if (humiditySensor.begin() == false)
  {
    Serial.println("Device did not acknowledge! Freezing.");
    while(1);
  }
  Serial.println("AHT20 acknowledged.");
}

void loop()
{
  temperature = humiditySensor.getTemperature();
  humidity = humiditySensor.getHumidity();

  Serial.println();
  Serial.print("TEMP: ");
  Serial.print(temperature);
  Serial.print(" C\t");
  Serial.print("HUMID: ");
  Serial.print(humidity);
  Serial.println("% RH");
  Serial.println();

  //Wait three seconds
  delay(3000);
}
