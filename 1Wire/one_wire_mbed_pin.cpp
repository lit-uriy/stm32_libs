#include "one_wire_mbed_pin.h"


OneWireMbedPin::OneWireMbedPin(DigitalInOut apin)
    : NewOneWire()
    , OneWirePhy()
    , _pin(apin)
    , _status(StatusUnknown)
    , _devices(16)
{
    pinInit();
    syncroPin.write(0);
}



