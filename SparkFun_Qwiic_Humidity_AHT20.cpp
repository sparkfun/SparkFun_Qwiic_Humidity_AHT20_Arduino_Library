/****************************************************************
 * SparkFun_Qwiic_Humidity_AHT20.cpp
 * SparkFun Qwiic Humidity AHT20 Library Source File
 * Priyanka Makin @ SparkFun Electronics
 * Original Creation Date: March 25, 2020
 * https://github.com/sparkfun/SparkFun_Qwiic_Humidity_AHT20_Arduino_Library
 * 
 * Pick up a board here: LINK GOES HERE
 * 
 * This file implements all the functions of the AHT20 class.
 * Functions in this library can return the humidity and temperature 
 * measured by the sensor. All data is communicated over I2C bus.
 * 
 * Development environment specifics:
 *  IDE: Arduino 1.8.9
 *  Hardware Platform: Arduino Uno
 *  Qwiic Humidity AHT20 Version: 1.0.0
 * 
 * This code is lemonade-ware; if you see me (or any other SparkFun
 * employee) at the local, and you've found our code helpful, please
 * buy us a round!
 * 
 * Distributed as-is; no warranty is given.
 ***************************************************************/

#include <SparkFun_Qwiic_Humidity_AHT20.h>

/*--------------------------- Device Status ------------------------------*/
bool AHT20::begin(uint8_t address, TwoWire &wirePort)
{
    _deviceAddress = address;   //Grab the address that the humidity sensor is on
    _i2cPort = &wirePort;   //Grab the port the user wants to communicate on

    //DEBUGGING: do we need to do this?!
    //We need to wait 20 ms at power on for sensor to receive commands
    //Datasheet pg 7
    delay(20);

    if (isConnected() == false)
        return false;

    return true;
}

//Ping the AHT20's I2C address
//If we get a response, we are correctly communicating with the AHT20
bool AHT20::isConnected()
{
    _i2cPort->beginTransmission(_deviceAddress);
    if (_i2cPort->endTransmission() != 0)
        return false;
    return true;
}

/*------------------------ Measurement Helpers ---------------------------*/

uint8_t AHT20::getStatus()
{
    uint8_t stat;
    read(STATUS, (uint8_t*)&stat, sizeof(stat));
    return stat;
}

//Returns the state of the cal bit in the status byte
bool AHT20::checkCalBit(uint8_t stat)
{
    //Check that the third status bit is a 1
    //The third bit is the CAL enable bit
    uint8_t temp = 1;
    temp = temp << 3;
    if (stat && temp)
        return true;    //AHT20 has been callibrated
    return false;   //AHT20 has not been callibrated
}

//Returns the state of the busy bit in the status byte
bool AHT20::checkBusyBit(uint8_t stat)
{
    //Check that the seventh status bit is a 1
    //The seventh bit is the busy indication bit
    uint8_t temp = 1;
    temp = temp << 7;
    if (stat && temp)
        return true;    //AHT20 is busy taking a measurement
    return false;   //AHT20 is not busy
}

bool AHT20::initialize()
{   
    //init comes from the datatsheet pg 8
    uint16_t init = 0x0800;
    return write(INITIALIZATION, (uint8_t*)&init, sizeof(init));
}

bool AHT20::triggerMeasurement()
{
    //trigMeas comes from the datasheet pg 8
    uint16_t trigMeas = 0x3300;
    return write(MEASUREMENT, (uint8_t*)&trigMeas, sizeof(trigMeas));
}

//Read and return six bytes of data
long AHT20::readData()
{
    long data;
    //Send status byte - datasheet pg 8
    read(STATUS, (uint8_t*)&data, sizeof(data));
    return data;
}

bool AHT20::softReset()
{
    return writeSingle(RESET);
}

//TODO
float AHT20::calculateTemperature(long data)
{
    //Parse out the state
    //Parse out the temperature bytes
    //Convert from raw bytes to degrees celcius
}

//TODO
float AHT20::calculateHumidity(long data)
{
    //Parse out the state
    //Parse out the humidity bytes
    //Convert from raw bytes to % RH
}

/*------------------------- Make Measurements ----------------------------*/

float AHT20::getTemperature()
{
    //wait 40 ms - datasheet pg 8
    delay(40);

    //Check the calibration bit of the status byte
    uint8_t status;
    status = getStatus();
    if (checkCalBit(status))
    {
        //Continue with the measurement sequence

        //Send initialization bytes
        initialize();

        //Signal ready to take measurement
        triggerMeasurement();
        
        //wait 75 ms for measurement to complete - datasheet pg 8
        delay(75);

        //Get status again and check that AHT20 is NOT still busy measuring
        status = getStatus();
        if (!checkBusyBit(status))
        {
            //Continue with measurement sequence

            //Read next six bytes (data)
            long data = readData();

            //Convert to temperature in celcius
            float temp = calculateTemperature(data);
            
            return temp;
        }
    }

    //Else, fail
    return 0;
}

float AHT20::getHumidity()
{
    //Wait 40 ms - datasheet pg 8
    delay(40);

    //Check the calibration bit of the status byte
    uint8_t status;
    status = getStatus();
    if (checkCalBit(status))
    {
        //Continue with the measurement sequence

        //Send the initialization bytes
        initialize();

        //Signal ready to take measurement
        triggerMeasurement();

        //wait 75 ms for measurement to complete - datasheet pg 8
        delay(75);

        //Get status again and check that AHT20 is NOT still busy measuring
        status = getStatus();
        if (!checkBusyBit(status))
        {
            //Continue with measurement sequence

            //Read next six bytes (data)
            long data = readData();

            //Convert to % RH  
            float humidity = calculateHumidity(data);

            return humidity;
        }
    }

    //Else, fail
    return 0;
}

/*-------------------------- I2C Abstraction -----------------------------*/

bool AHT20::read(uint8_t key, uint8_t *buff, uint8_t buffSize)
{
    _i2cPort-> beginTransmission(_deviceAddress);
    _i2cPort->write(key);
    _i2cPort->endTransmission();

    if (_i2cPort->requestFrom(_deviceAddress, buffSize) > 0)
    {
        for (uint8_t i = 0; i < buffSize; i++)
        {
            buff[i] = _i2cPort->read();
        }
        return true;
    }
    return false;
}

bool AHT20::write(uint8_t key, uint8_t *buff, uint8_t buffSize)
{
    _i2cPort->beginTransmission(_deviceAddress);
    _i2cPort->write(key);

    for (uint8_t i = 0; i < buffSize; i++)
        _i2cPort->write(buff[i]);
    
    if (_i2cPort->endTransmission() == 0)
        return true;
    return false;
}

bool AHT20::writeSingle(uint8_t key)
{
    _i2cPort->beginTransmission(_deviceAddress);
    _i2cPort->write(key);

    if (_i2cPort->endTransmission() == 0)
        return true;
    return false;
}