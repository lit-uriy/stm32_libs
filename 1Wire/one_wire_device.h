#ifndef ONEWIREDEVICE_H
#define ONEWIREDEVICE_H


#include "one_wire.h"


class OneWireDevice
{
public:
    OneWireDevice(unsigned char *aRomCode = 0);

    unsigned char familyCode();

protected:
    bool readROM(unsigned char aRomCode[]);
    bool matchROM(const unsigned char aRomCode[]);
    void searchROM();
    void skipROM();

    OneWire::LineStatus readWriteByte(unsigned char *byte);
    unsigned char crc8(unsigned char data, unsigned char crc8val);

    unsigned char *_romCode;

    OneWire* wire() {return _wire;}

private:
    OneWire *_wire;

    friend class OneWire;
};

#endif // ONEWIREDEVICE_H
