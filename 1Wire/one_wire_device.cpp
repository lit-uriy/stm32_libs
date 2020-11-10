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

void OneWireDevice::appliedCommand(unsigned char command, OneWireRomCode *romCode){
    unsigned char i;

    if (romCode) {
        matchROM(*romCode);  // to a single device
    } else {
        skipROM();          // to all devices
    }

    _wire->readWriteByte(&command);
}


