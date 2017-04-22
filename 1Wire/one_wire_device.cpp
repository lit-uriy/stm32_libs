#include "one_wire_device.h"

OneWireDevice::OneWireDevice(unsigned char *aRomCode)
    : wire(0)
{

}


bool OneWireDevice::readROM(unsigned char aRomCode[])
{
    if (!wire)
        return false;
    return wire->readROM(aRomCode);
}

void OneWireDevice::matchROM(const unsigned char aRomCode[])
{
    if (!wire)
        return;
    wire->matchROM(aRomCode);
}

void OneWireDevice::searchROM()
{
    if (!wire)
        return;
    wire->searchROM();
}

void OneWireDevice::skipROM()
{
    if (!wire)
        return;
    wire->skipROM();
}

OneWire::LineStatus OneWireDevice::readWriteByte(unsigned char *byte)
{
    if (!wire)
        return OneWire::StatusUnknown;
    return wire->readWriteByte(byte);
}
