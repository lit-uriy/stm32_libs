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


public:
    OneWire::LineStatus searchROM(OneWireRomCode *romCode, bool next = true);
    bool matchROM(const OneWireRomCode romCode);
    bool skipROM();
    bool readROM(OneWireRomCode *romCode);

    void appliedCommand(unsigned char command, OneWireRomCode *romCode = 0);

protected:

    bool _valid;
    OneWireRomCode _romCode;

private:
    OneWire *_wire;

    friend class OneWire;
};

#endif // ONEWIREDEVICE_H
