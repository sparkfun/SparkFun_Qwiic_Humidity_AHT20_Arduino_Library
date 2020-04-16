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

    //Wait 40 ms after power-on before reading temp or humidity. Datasheet pg 8
    delay(40);

    //Check if the calibrated bit is set. If not, init the sensor.
    if (isCalibrated() == false)
    {
        //Send 0xBE0800
        initialize();

        //Immediately trigger a measurement. Send 0xAC3300
        triggerMeasurement();

        delay(75); //Wait for measurement to be complete
        while (isBusy())
            delay(1);
        if (isCalibrated() == false)
        {
            Serial.println("Calibration failed at startup");
            while (1)
                ;
        }
    }

    //Check that the cal bit has been set
    if (isCalibrated() == false)
    {
        return false;
    }

    //Mark all datums as fresh (not read before)
    sensorQueried.temperature = true;
    sensorQueried.humidity = true;

    return true;
}

//Ping the AHT20's I2C address
//If we get a response, we are correctly communicating with the AHT20
bool AHT20::isConnected()
{
    _i2cPort->beginTransmission(_deviceAddress);
    if (_i2cPort->endTransmission() == 0)
        return true;

    //If IC failed to respond, give it 20ms more for Power On Startup
    //Datasheet pg 7
    delay(20);

    if (_i2cPort->endTransmission() == 0)
        return true;

    return false;
}

/*------------------------ Measurement Helpers ---------------------------*/

uint8_t AHT20::getStatus()
{
    _i2cPort->requestFrom(_deviceAddress, (uint8_t)1);
    if (_i2cPort->available())
        return (_i2cPort->read());
    return (0);
}

//Returns the state of the cal bit in the status byte
bool AHT20::isCalibrated()
{
    return (getStatus() & (1 << 3));
}

//Returns the state of the busy bit in the status byte
bool AHT20::isBusy()
{
    uint8_t status = getStatus();

    //Check that the seventh status bit is a 1
    //The seventh bit is the busy indication bit
    uint8_t temp = 1;
    temp = temp << 7;
    if (status & temp)
        return true; //AHT20 is busy taking a measurement
    return false;    //AHT20 is not busy
}

bool AHT20::initialize()
{
    _i2cPort->beginTransmission(_deviceAddress);
    _i2cPort->write(sfe_aht20_reg_initialize);
    _i2cPort->write(0x80);
    _i2cPort->write(0x00);
    if (_i2cPort->endTransmission() == 0)
        return true;
    return false;
}

bool AHT20::triggerMeasurement()
{
    uint8_t command[2] = {0x33, 0x00};

    _i2cPort->beginTransmission(_deviceAddress);
    _i2cPort->write(sfe_aht20_reg_measure);
    _i2cPort->write(0x33);
    _i2cPort->write(0x00);
    if (_i2cPort->endTransmission() == 0)
        return true;
    return false;
}

//Loads the
void AHT20::readData()
{
    //Clear previous data
    sensorData.temperature = 0;
    sensorData.humidity = 0;

    if (_i2cPort->requestFrom(_deviceAddress, (uint8_t)6) > 0)
    {
        uint8_t state = _i2cPort->read();

        uint32_t incoming = 0;
        incoming |= (uint32_t)_i2cPort->read() << (8 * 2);
        incoming |= (uint32_t)_i2cPort->read() << (8 * 1);
        uint8_t midByte = _i2cPort->read();

        incoming |= midByte;
        sensorData.humidity = incoming >> 4;

        sensorData.temperature = (uint32_t)midByte << (8 * 2);
        sensorData.temperature |= (uint32_t)_i2cPort->read() << (8 * 1);
        sensorData.temperature |= (uint32_t)_i2cPort->read() << (8 * 0);

        //Need to get rid of data in bits > 20
        sensorData.temperature = sensorData.temperature & ~(0xFFF00000);

        //Mark data as fresh
        sensorQueried.temperature = false;
        sensorQueried.humidity = false;
    }
}

// //Returns temperature in degrees celcius
// float AHT20::calculateTemperature(dataStruct data)
// {
//     float tempCelsius;
//     //From datasheet pg 8
//     tempCelsius = ((float)data.temperature / 1048576) * 200 - 50;

//     // //DEBUGGING
//     // float relHumidity;
//     // relHumidity = ((float)data.humidity / 1048576) * 100;
//     // //Print the result
//     // Serial.print("Temperature: ");
//     // Serial.print(tempCelcius);
//     // Serial.print(" C \t Humidity: ");
//     // Serial.print(relHumidity);
//     // Serial.println("% RH");
//     // Serial.println();

//     return tempCelsius;
// }

// //Returns humidity in %RH
// float AHT20::calculateHumidity(dataStruct data)
// {
//     float relHumidity;
//     //From datasheet pg 8
//     relHumidity = ((float)data.humidity / 1048576) * 100;
//     return relHumidity;
// }

//Triggers a measurement if one has not been previously started, the returns false
//If measurement has been started, checks to see if complete.
//If not complete, returns false
//If complete, readData(), return true, mark measurement as not started
bool AHT20::available()
{
    if (measurementStarted == false)
    {
        triggerMeasurement();
        measurementStarted = true;
        return (false);
    }

    if (isBusy() == true)
    {
        return (false);
    }

    readData();
    measurementStarted = false;
    return (true);
}

bool AHT20::softReset()
{
    _i2cPort->beginTransmission(_deviceAddress);
    _i2cPort->write(sfe_aht20_reg_reset);
    if (_i2cPort->endTransmission() == 0)
        return true;
    return false;
}

/*------------------------- Make Measurements ----------------------------*/

float AHT20::getTemperature()
{
    if (sensorQueried.temperature == true)
    {
        //We've got old data. Go get new
        //Trigger measurement
        triggerMeasurement();
        delay(100);
        if (isBusy() == false)
        {
            //Continue
            // Serial.println("AHT20 not busy. Continue.");

            readData();
        }
    }

    //From datasheet pg 8
    float tempCelsius = ((float)sensorData.temperature / 1048576) * 200 - 50;

    //Mark data as old
    sensorQueried.temperature = true;

    return tempCelsius;
}

float AHT20::getHumidity()
{
    float humidity;

    // //Wait 40 ms - datasheet pg 8
    // delay(40);

    // //Check calibration bit of status byte
    // uint8_t status;
    // status = getStatus();
    // //DEBUGGING
    // // Serial.print("State: 0x");
    // // Serial.println(status, HEX);
    // if (isCalibrated())
    // {
    //     //Continue
    //     // Serial.println("AHT20 callibrated!");

    //     //Initialize
    //     initialize();
    //     Serial.println("AHT20 has been initialized.");

    //     //Trigger measurement
    //     triggerMeasurement();
    //     Serial.println("AHT20 measurement has been triggered.");

    //     //Wait 100 ms
    //     //DEBUG: turn this into an available() function??
    //     Serial.println("Wait 100 ms");
    //     delay(400);

    //     //Check the busy bit
    //     status = getStatus();
    //     //DEBUGGING
    //     Serial.print("State: 0x");
    //     Serial.println(status, HEX);
    //     if (isBusy() == false)
    //     {
    //         //Continue
    //         Serial.println("AHT20 not busy. Continue.");

    //         raw_data newData = readData();
    //         humidity = calculateHumidity(newData);
    //     }
    //     else
    //     {
    //         Serial.println("Can't continue. AHT20 indicating busy taking measurement. Freezing.");
    //         while (1)
    //             ;
    //     }
    // }
    // else
    // {
    //     Serial.println("Chip not callibrated! Freezing.");
    //     while (1)
    //         ;
    // }

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