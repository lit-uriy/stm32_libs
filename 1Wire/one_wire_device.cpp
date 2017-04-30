#include "one_wire_device.h"

#include "../utils/crc.h"

OneWireDevice::OneWireDevice(OneWireRomCode aRomCode, OneWire *awire)
    : _romCode(aRomCode)
    , _wire(awire)
{

}

unsigned char OneWireDevice::familyCode()
{
    return _romCode.bytes[0];
}


