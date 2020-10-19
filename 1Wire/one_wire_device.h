#ifndef ONEWIREDEVICE_H
#define ONEWIREDEVICE_H


#include "one_wire.h"


class OneWireDevice
{
public:
    OneWireDevice(OneWireRomCode aRomCode, OneWire *awire = 0);
    virtual ~OneWireDevice(){}

    bool isValid(){return _valid;}

    OneWireRomCode romCode();
    unsigned char familyCode();

    OneWire* wire() {return _wire;}

protected:

    bool _valid;
    OneWireRomCode _romCode;

private:
    OneWire *_wire;

    friend class OneWire;
};

#endif // ONEWIREDEVICE_H
