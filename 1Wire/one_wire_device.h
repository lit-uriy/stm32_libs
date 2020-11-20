#ifndef ONEWIREDEVICE_H
#define ONEWIREDEVICE_H


#include "new_one_wire.h"


class OneWireDevice
{
public:
    OneWireDevice(OneWireRomCode aRomCode, NewOneWire *awire = 0);
    virtual ~OneWireDevice(){}

    bool isValid(){return _valid;}

    OneWireRomCode romCode();
    unsigned char familyCode();

    NewOneWire* wire() {return _wire;}


public:
//    OneWire::LineStatus searchROM(OneWireRomCode *romCode, bool next = true);
//    bool matchROM(const OneWireRomCode romCode);
//    bool skipROM();
//    bool readROM(OneWireRomCode *romCode);

    // Прикладная команда этому устройству
    bool appliedCommand(unsigned char command);

    // Прикладная команда всем устройствам на проволоке
    static bool appliedCommand(unsigned char command, NewOneWire *awire);

protected:

    bool _valid;
    OneWireRomCode _romCode;

private:
    NewOneWire *_wire;

    friend class NewOneWire;
};

#endif // ONEWIREDEVICE_H
