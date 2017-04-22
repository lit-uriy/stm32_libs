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
};

#endif // YDS1820_H
