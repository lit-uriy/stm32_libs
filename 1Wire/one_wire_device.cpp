#include "one_wire_device.h"

#include "../utils/crc.h"

extern DigitalOut syncroPin;

OneWireDevice::OneWireDevice(OneWireRomCode aRomCode, OneWire *awire)
    : _valid(true)
    , _romCode(aRomCode)
    , _wire(awire)
{
//    syncroPin.write(0);

}

unsigned char OneWireDevice::familyCode()
{
    return _romCode.bytes[0];
}

bool OneWireDevice::appliedCommand(unsigned char command, OneWire *awire)
{
    if (!awire)
        return false;

    awire->skipROM();          // to all devices

    awire->readWriteByte(&command);
    return true;
}

bool OneWireDevice::appliedCommand(unsigned char command)
{
    if (!_wire)
        return false;

    _wire->matchROM(_romCode);  // to a single device

    _wire->readWriteByte(&command);
    return true;
}


