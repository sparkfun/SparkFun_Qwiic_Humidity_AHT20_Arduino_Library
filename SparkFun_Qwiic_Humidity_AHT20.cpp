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
    _deviceAddress = address; //Grab the address that the humidity sensor is on
    _i2cPort = &wirePort;     //Grab the port the user wants to communicate on

    if (isConnected() == false)
        return false;

    return true;
}

//Ping the AHT20's I2C address
//If we get a response, we are correctly communicating with the AHT20
bool AHT20::isConnected()
{
    _i2cPort->beginTransmission(_deviceAddress);
    if (_i2cPort->endTransmission() == 0)
        return true;

    //If IC failed to respond, give it 20ms more for Power On startup
    //Datasheet pg 7
    delay(20);

    if (_i2cPort->endTransmission() == 0)
        return true;

    return false;
}

/*------------------------ Measurement Helpers ---------------------------*/

uint8_t AHT20::getStatus()
{
    uint8_t stat;
    _i2cPort->requestFrom(_deviceAddress, 1);
    while (_i2cPort->available())
    {
        stat = _i2cPort->read();
    }
    return stat;
}

//Returns the state of the cal bit in the status byte
bool AHT20::checkCalBit(uint8_t stat)
{
    //Check that the third status bit is a 1
    //The third bit is the CAL enable bit
    uint8_t temp = 1;
    temp = temp << 3;
    if (stat & temp)
        return true; //AHT20 has been callibrated
    return false;    //AHT20 has not been callibrated
}

//Returns the state of the busy bit in the status byte
bool AHT20::checkBusyBit(uint8_t stat)
{
    //Check that the seventh status bit is a 1
    //The seventh bit is the busy indication bit
    uint8_t temp = 1;
    temp = temp << 7;
    if (stat & temp)
        return true; //AHT20 is busy taking a measurement
    return false;    //AHT20 is not busy
}

bool AHT20::initialize()
{
    _i2cPort->beginTransmission(_deviceAddress);
    _i2cPort->write(INITIALIZATION);
    _i2cPort->write((uint8_t *)INIT_CMD, 2);
    if (_i2cPort->endTransmission() == 0)
        return true;
    return false;
}

bool AHT20::triggerMeasurement()
{
    _i2cPort->beginTransmission(_deviceAddress);
    _i2cPort->write(MEASUREMENT);
    _i2cPort->write((uint8_t *)MEAS_CMD, 2);
    if (_i2cPort->endTransmission() == 0)
        return true;
    return false;
}

//Read and return six bytes of data
dataStruct AHT20::readData()
{
    raw_data data;

    if (_i2cPort->requestFrom(_deviceAddress, 6) > 0)
    {
        uint8_t state = _i2cPort->read();
        // Serial.print("State: 0x");
        // Serial.println(state, HEX);

        uint32_t incoming = 0;
        incoming |= (uint32_t)_i2cPort->read() << (8 * 2);
        // Serial.println(incoming, HEX);
        incoming |= (uint32_t)_i2cPort->read() << (8 * 1);
        // Serial.println(incoming, HEX);
        uint8_t midByte = _i2cPort->read();

        incoming |= midByte << (8 * 0);
        // Serial.println(incoming, HEX);
        data.humidity = incoming >> 4;
        // Serial.println(data.humidity, HEX);
        // Serial.println("Read-in humidity correct, I think?");

        data.temperature = (uint32_t)midByte << (8 * 2);
        // Serial.println(data.temperature, HEX);
        data.temperature |= (uint32_t)_i2cPort->read() << (8 * 1);
        // Serial.println(data.temperature, HEX);
        data.temperature |= (uint32_t)_i2cPort->read() << (8 * 0);
        // Serial.println(data.temperature, HEX);

        //Need to get rid of data in bits > 20
        data.temperature = data.temperature & ~(0xFFF00000);
        // Serial.println(data.temperature, HEX);
        // Serial.println("Read-in temperature correct, too.");
    }

    // Serial.println("Read-in AHT20 raw data");
    // Serial.print("Raw temp: 0x");
    // Serial.println(data.temperature, HEX);
    // Serial.print("Raw humidity: 0x");
    // Serial.println(data.humidity, HEX);
    // Serial.println();

    return data;
}

//Returns temperature in degrees celcius
float AHT20::calculateTemperature(dataStruct data)
{
    float tempCelcius;
    //From datasheet pg 8
    tempCelcius = ((float)data.temperature / 1048576) * 200 - 50;

    // //DEBUGGING
    // float relHumidity;
    // relHumidity = ((float)data.humidity / 1048576) * 100;
    // //Print the result
    // Serial.print("Temperature: ");
    // Serial.print(tempCelcius);
    // Serial.print(" C \t Humidity: ");
    // Serial.print(relHumidity);
    // Serial.println("% RH");
    // Serial.println();

    return tempCelcius;
}

//Returns humidity in %RH
float AHT20::calculateHumidity(dataStruct data)
{
    float relHumidity;
    //From datasheet pg 8
    relHumidity = ((float)data.humidity / 1048576) * 100;
    return relHumidity;
}

bool AHT20::softReset()
{
    _i2cPort->beginTransmission(_deviceAddress);
    _i2cPort->write(RESET);
    if (_i2cPort->endTransmission() == 0)
        return true;
    return false;
}

/*------------------------- Make Measurements ----------------------------*/

float AHT20::getTemperature()
{
    float temperature;

    //Wait 40 ms - datasheet pg 8
    delay(40);

    //Check calibration bit of status byte
    uint8_t status;
    status = getStatus();
    //DEBUGGING
    // Serial.print("State: 0x");
    // Serial.println(status, HEX);
    if (checkCalBit(status))
    {
        //Continue
        // Serial.println("AHT20 callibrated!");

        //Initialize
        initialize();
        // Serial.println("AHT20 has been initialized.");

        //Trigger measurement
        triggerMeasurement();
        // Serial.println("AHT20 measurement has been triggered.");

        //Wait 100 ms
        //DEBUG: turn this into an available() function??
        // Serial.println("Wait 100 ms");
        delay(400);

        //Check the busy bit
        status = getStatus();
        //DEBUGGING
        // Serial.print("State: 0x");
        // Serial.println(status, HEX);
        if (!checkBusyBit(status))
        {
            //Continue
            // Serial.println("AHT20 not busy. Continue.");

            raw_data newData = readData();
            temperature = calculateTemperature(newData);
        }
        else
        {
            // Serial.println("Can't continue. AHT20 indicating busy taking measurement. Freezing.");
            while (1)
                ;
        }
    }
    else
    {
        // Serial.println("Chip not callibrated! Freezing.");
        while (1)
            ;
    }

    return temperature;
}

float AHT20::getHumidity()
{
    float humidity;

    //Wait 40 ms - datasheet pg 8
    delay(40);

    //Check calibration bit of status byte
    uint8_t status;
    status = getStatus();
    //DEBUGGING
    // Serial.print("State: 0x");
    // Serial.println(status, HEX);
    if (checkCalBit(status))
    {
        //Continue
        // Serial.println("AHT20 callibrated!");

        //Initialize
        initialize();
        Serial.println("AHT20 has been initialized.");

        //Trigger measurement
        triggerMeasurement();
        Serial.println("AHT20 measurement has been triggered.");

        //Wait 100 ms
        //DEBUG: turn this into an available() function??
        Serial.println("Wait 100 ms");
        delay(400);

        //Check the busy bit
        status = getStatus();
        //DEBUGGING
        Serial.print("State: 0x");
        Serial.println(status, HEX);
        if (!checkBusyBit(status))
        {
            //Continue
            Serial.println("AHT20 not busy. Continue.");

            raw_data newData = readData();
            humidity = calculateHumidity(newData);
        }
        else
        {
            Serial.println("Can't continue. AHT20 indicating busy taking measurement. Freezing.");
            while (1)
                ;
        }
    }
    else
    {
        Serial.println("Chip not callibrated! Freezing.");
        while (1)
            ;
    }

    return humidity;
}

// float AHT20::getTemperature()
// {
//     //wait 40 ms - datasheet pg 8
//     delay(40);

//     //Check the calibration bit of the status byte
//     uint8_t status;
//     status = getStatus();
//     if (checkCalBit(status))
//     {
//         //Continue with the measurement sequence

//         //Send initialization bytes
//         initialize();

//         //Signal ready to take measurement
//         triggerMeasurement();

//         //wait 100 ms for measurement to complete - datasheet pg 8
//         delay(100);

//         //Get status again and check that AHT20 is NOT still busy measuring
//         status = getStatus();
//         if (!checkBusyBit(status))
//         {
//             //Continue with measurement sequence

//             //Read next six bytes (data)
//             raw_data data = readData();

//             //Convert to temperature in celcius
//             float temp = calculateTemperature(data);

//             return temp;
//         }
//     }

//     Serial.println("I've failed!");

//     //Else, fail
//     return 0;
// }

// float AHT20::getHumidity()
// {
//     //Wait 40 ms - datasheet pg 8
//     delay(40);

//     //Check the calibration bit of the status byte
//     uint8_t status;
//     status = getStatus();
//     if (checkCalBit(status))
//     {
//         //Continue with the measurement sequence

//         //Send the initialization bytes
//         initialize();

//         //Signal ready to take measurement
//         triggerMeasurement();

//         //wait 100 ms for measurement to complete - datasheet pg 8
//         delay(100);

//         //Get status again and check that AHT20 is NOT still busy measuring
//         status = getStatus();
//         if (!checkBusyBit(status))
//         {
//             //Continue with measurement sequence

//             //Read next six bytes (data)
//             raw_data data = readData();

//             //Convert to % RH
//             float humidity = calculateHumidity(data);

//             return humidity;
//         }
//     }

//     Serial.println("I've failed!");

//     //Else, fail
//     return 0;
// }