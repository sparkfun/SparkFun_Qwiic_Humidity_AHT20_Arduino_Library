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
 ******************************************************************/

#ifndef __SparkFun_Qwiic_Humidity_AHT20_H__
#define __SparkFun_Qwiic_Humidity_AHT20_H__

#include <Arduino.h>
#include <Wire.h>

#define DEFAULT_ADDRESS 0x38

enum registers
{
    sfe_aht20_reg_reset = 0xBA,
    sfe_aht20_reg_initialize = 0xBE,
    sfe_aht20_reg_measure = 0xAC,
};

struct sfe_aht20_raw_data
{
    uint32_t humidity, temperature;
};

typedef struct raw_data dataStruct;

class AHT20
{
private:
    TwoWire *_i2cPort; //The generic connection to user's chosen I2C hardware
    uint8_t _deviceAddress;

    sfe_aht20_raw_data sensorData;

public:
    //Device status
    bool begin(uint8_t address = DEFAULT_ADDRESS, TwoWire &wirePort = Wire); //Sets the address of the device and opens the I2C bus
    bool isConnected();                                                      //Checks if the AHT20 is connected to the I2C bus
    //DEBUG: check ID??

    //Measurement helper functions
    uint8_t getStatus();                         //Returns the status byte
    bool isCalibrated();                         //Returns true if the cal bit is set, false otherwise
    bool isBusy();                               //Returns true if the busy bit is set, false otherwise
    bool initialize();                           //Initialize for taking measurement
    bool triggerMeasurement();                   //Trigger the AHT20 to take a measurement
    void readData();                             //Read and parse the 6 bytes of data into raw humidity and temp
    float calculateTemperature(dataStruct data); //Convert raw bytes to temperature in celcius
    float calculateHumidity(dataStruct data);    //Convert raw bytes to relative humidity percentage
    bool softReset();                            //Restart the sensor system without turning power off and on

    //Make measurements
    float getTemperature(); //Goes through the measurement sequence and returns temperature in degrees celcius
    float getHumidity();    //Goes through the measurement sequence and returns humidity in % RH
};
#endif