#include "one_wire_mbed_pin.h"


OneWireMbedPin::OneWireMbedPin(DigitalInOut apin)
    : NewOneWire()
    , _pin(apin)
{
    pinInit();
}



