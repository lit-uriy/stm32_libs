#include "one_wire_device.h"

OneWireDevice::OneWireDevice(unsigned char *aRomCode)
    : _romCode(aRomCode)
    , _wire(0)
{

}

unsigned char OneWireDevice::familyCode()
{
    return _romCode[0];
}

bool OneWireDevice::readROM(unsigned char aRomCode[])
{
    if (!_wire)
        return false;
    return _wire->readROM(aRomCode);
}

bool OneWireDevice::matchROM(const unsigned char aRomCode[])
{
    if (!_wire)
        return false;
    return _wire->matchROM(aRomCode);
}

void OneWireDevice::searchROM()
{
    if (!_wire)
        return;
    _wire->searchROM();
}

void OneWireDevice::skipROM()
{
    if (!_wire)
        return;
    _wire->skipROM();
}

OneWire::LineStatus OneWireDevice::readWriteByte(unsigned char *byte)
{
    if (!_wire)
        return OneWire::StatusUnknown;
    return _wire->readWriteByte(byte);
}


unsigned char OneWireDevice::crc8(unsigned char data, unsigned char crc8val)
{
    if (!_wire)
        return 0xFF;
    return _wire->crc8(data, crc8val);
}

