#ifndef ONEWIREDEVICE_H
#define ONEWIREDEVICE_H


#include "one_wire.h"


class OneWireDevice
{
public:
    OneWireDevice(OneWireRomCode aRomCode, OneWire *awire = 0);
    virtual ~OneWireDevice(){}

    unsigned char familyCode();


    bool romString(char buff[]);

protected:

    OneWireRomCode _romCode;

    OneWire* wire() {return _wire;}

private:
    OneWire *_wire;

    friend class OneWire;
};

#endif // ONEWIREDEVICE_H
