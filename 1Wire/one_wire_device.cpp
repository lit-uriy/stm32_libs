#include "one_wire_device.h"

#include "../utils/crc.h"

extern DigitalOut syncroPin;

OneWireDevice::OneWireDevice(OneWireRomCode aRomCode, OneWire *awire)
    : _romCode(aRomCode)
    , _wire(awire)
{
//    syncroPin.write(0);
}

unsigned char OneWireDevice::familyCode()
{
    return _romCode.bytes[0];
}


