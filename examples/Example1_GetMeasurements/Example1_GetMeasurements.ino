/**************************************************************************
   Tests the getTemperature and getHumidity functions of the Qwiic Humidity
   library

   Priyanka Makin @ SparkFun Electronics
   Original Creation Date: March 31, 2020

   SparkFun labored with love to create this code. Feel like supporting open
   source hardware? Buy a board from SparkFun! https://www.sparkfun.com/products/16618

   This code is lemonadeware; if you see me (or any other SparkFun employee)
   at the local, and you've found our code helpful, please buy us a round!

   Hardware Connections:
   Attach a RedBoard to computer using micro-B USB cable.
   Attach a Qwiic Humidity board to RedBoard using Qwiic cable.

   Distributed as-is; no warranty is given.
 **************************************************************************/
#include <Wire.h>

#include <SparkFun_Qwiic_Humidity_AHT20.h> //Click here to get the library: http://librarymanager/All#Qwiic_Humidity_AHT20 by SparkFun
AHT20 humiditySensor;

void setup()
{
  Serial.begin(115200);
  Serial.println("Qwiic Humidity AHT20 examples");

  Wire.begin(); //Join I2C bus

  //Check if the AHT20 will acknowledge
  if (humiditySensor.begin() == false)
  {
    Serial.println("AHT20 not detected. Please check wiring. Freezing.");
    while (1);
  }
  Serial.println("AHT20 acknowledged.");
}

void loop()
{
  //If a new measurement is available
  if (humiditySensor.available() == true)
  {
    //Get the new temperature and humidity value
    float temperature = humiditySensor.getTemperature();
    float humidity = humiditySensor.getHumidity();

    //Print the results
    Serial.print("Temperature: ");
    Serial.print(temperature, 2);
    Serial.print(" C\t");
    Serial.print("Humidity: ");
    Serial.print(humidity, 2);
    Serial.print("% RH");

    Serial.println();
  }

  //The AHT20 can respond with a reading every ~50ms. However, increased read time can cause the IC to heat around 1.0C above ambient.
  //The datasheet recommends reading every 2 seconds.
  delay(2000);
}
