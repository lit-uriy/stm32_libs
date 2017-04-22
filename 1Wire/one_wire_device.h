#ifndef ONEWIREDEVICE_H
#define ONEWIREDEVICE_H


#include "one_wire.h"


class OneWireDevice
{
public:
    OneWireDevice(unsigned char *aRomCode = 0);

protected:
    bool readROM(unsigned char aRomCode[]);
    void matchROM(const unsigned char aRomCode[]);
    void searchROM();
    void skipROM();

    OneWire::LineStatus readWriteByte(unsigned char *byte);

private:
    OneWire *wire;

    friend class OneWire;
};

#endif // ONEWIREDEVICE_H
