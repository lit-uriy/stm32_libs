#ifndef YDS1820_H
#define YDS1820_H

#include "one_wire_device.h"

class Yds1820: public OneWireDevice
{
public:
    enum Devices{
        DeviceThis,     // command applies to only this device
        DevicesAll   // command applies to all devices
    };

    enum {
        invalid_conversion = -1000
    };

    Yds1820(unsigned char *aRomCode = 0);

    int convertTemperature(bool wait, Devices device=DevicesAll);

    float temperature(char scale='c');

    bool setResolution(unsigned int resolution);


    enum DeviceCommands {
        CommandAlarmSearch = 0xEC, // числится как ROM-команда в PDF-нике, но не в книге по iButton
        // DS18B20 Function Commands
        CommandConvertT = 0x44,
        CommandWriteScratchpad = 0x4E,
        CommandReadScratchpad = 0xBE,
        CommandCopyScratchpad = 0x48,
        CommandRecallEeprom = 0xB8,
        CommandReadPowerSupply = 0xB4,
    };
};

#endif // YDS1820_H
