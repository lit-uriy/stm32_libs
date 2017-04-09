#include "one_wire.h"

OneWire::OneWire(DigitalInOut apin)
    : _pin(apin)
{

}

LineStatus OneWire::reset()
{
    // в начале на шине должна быть "1"
    if (!pin())
        return StatusShortCircuit;
    // выставляем "Reset pulse"
    pinLow();
    // выдержваем в течении TRSTL
    deleyUs(TimeReset);

    // снимаем "Reset pulse"
    pinRelease();
    // ждем возврата линии "0"->"1" (слэйв подождёт TimePdh, а затем выставит Presence на время TimePdl)
    deleyUs(TimeRelease);
    // на шине должна стать "1"
    if (!pin())
        return StatusShortCircuit;

    // подождем время, за которое слэйв гарантировано выставит "Presence pulse"
    deleyUs(TimeSlot);
    // проверим наличие "Presence pulse"
    if (pin())
        return StatusAbsent;

    //ждем завершения "Presence pulse" "0"->"1"
    deleyUs(TimePresence);
    // на шине должна стать "1"
    if (!pin())
        return StatusShortCircuit;

    // нормальный "Presence pulse" был
    return StatusPresence;
}

// 0x33 или 0x0F - старая таблетка DS1990 (без буквы А)
void OneWire::readROM()
{

}

// 0x55
void OneWire::matchROM()
{

}

// 0xF0
void OneWire::searchROM()
{

}

// 0xCC
void OneWire::skipROM()
{

}

