/****************************************************************
 * SparkFun_Qwiic_Humidity_AHT20.h
 * SparkFun Qwiic Humidity AHT20 Library Header File
 * Priyanka Makin @ SparkFun Electronics
 * Original Creation Date: March 25, 2020
 * https://github.com/sparkfun/SparkFun_Qwiic_Humidity_AHT20_Arduino_Library
 * 
 * Pickup a board here: LINK GOES HERE
 * 
 * This file prototypes the AHT20 class implemented in SparkFun_Qwiic_Humidity_AHT20.cpp
 * 
 * Development environment specifics:
 *  IDE: Arduino 1.8.9
 *  Hardware Platform: Arduino Uno
 *  Qwiic Humidity AHT20 Version: 1.0.0
 * 
 * This code is lemonade-ware; if you see me (or any other SparkFun employee)
 * at the local, and you've found our code helpful, please buy us a round!
 * 
 * Distributed as-is; no warranty is given.
 ******************************************************************/
 
#ifndef __SparkFun_Qwiic_Humidity_AHT20_H__
#define __SparkFun_Qwiic_Humidity_AHT20_H__

#include <Arduino.h>
#include <Wire.h>

#define DEFAULT_ADDRESS 0x38

class AHT20:
{
    private:
        TwoWire *_i2cPort;  //The generic connection to user's chosen I2C hardware
        uint8_t _deviceAddress;

    public:
        //Device status
        bool begin(uint8_t address = DEFAULT_ADDRESS, TwoWire &wirePort = Wire);    //Sets the address of the device and opens the I2C bus
        bool isConnected();
        bool initialise();

        //Get measurements
        bool getTemperature();
        bool getHumidity();

        //I2C Abstraction
        bool read(uint8_t *buff, uint8_t buffSize);
        bool write(uint8_t *buff, uint8_t buffSize);
};

#endif