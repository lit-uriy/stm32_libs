#include "one_wire.h"

#include "1Wire/one_wire_device.h"

extern DigitalOut syncroPin;

extern bool test;

OneWire::OneWire(DigitalInOut apin)
    : _pin(apin)
    , _status(StatusUnknown)
    , _devices(16)
{
    syncroPin.write(0);

    // Line state: 1
    _pin.output();
    _pin.mode(OpenDrain); // Line state: 0
    pinRelease(); // Line state: 1
}

OneWire::LineStatus OneWire::status()
{
    return _status;
}


int OneWire::findSingleDevices()
{
    return 1; // FIXME: сделать реальную Search ROM
}

void OneWire::addDevice(OneWireDevice *dev)
{
    _devices.append(dev);
    dev->_wire = this;
}

void OneWire::removeDevice(OneWireDevice *dev)
{
    int index = _devices.indexOf(dev);
    if (!index)
        return;
    _devices.removeAt(index);
    dev->_wire = 0;
}

YList<OneWireDevice *> OneWire::devices()
{
    return _devices;
}

OneWire::LineStatus OneWire::reset()
{
    // в начале на шине должна быть "1"
    if (!pin()){
        _status = StatusShortCircuit;
        return _status;
    }

    // выставляем "Reset pulse"
    pinLow();
    // выдержваем в течении TRSTL
    deleyUs(TimeReset);

    // снимаем "Reset pulse"
    pinRelease();
    // ждем возврата линии "0"->"1" (слэйв подождёт TimePdh, а затем выставит Presence на время TimePdl)
    deleyUs(TimeRelease); // здесь можно сделать измерение времени восстановления
    // на шине должна стать "1"
    if (!pin()){
        _status = StatusShortCircuit;
        return _status;
    }

    // подождем время, за которое слэйв гарантировано выставит "Presence pulse"
    deleyUs(TimeSlot);
    // проверим наличие "Presence pulse"
    if (pin()){
        _status = StatusAbsent;
        return _status;
    }

    //ждем завершения "Presence pulse" "0"->"1"
    deleyUs(TimePresence);
    // на шине должна стать "1"
    if (!pin()){
        _status = StatusShortCircuit;
        return _status;
    }

    deleyUs(TimeReset - TimePresence); // общая выдержка в отпущенном состоянии не менее TimeReset

    // нормальный "Presence pulse" был
    _status = StatusPresence;
    return _status;
}

OneWire::LineStatus OneWire::readWriteByte(unsigned char *byte)
{
    unsigned char j;

    for(j=0;j<8;j++)
    {
        // должна быть "1"
        if (!pin()) {
            printf("Error ocured on before syncro front\r\n");
            _status = StatusShortCircuit;
            return _status;
        }
        //выставляем "Синхрофронт" на 10мкс, т.к. через 15 мкс слэйв будет читать данные

        pinLow();
        deleyUs(TimeSyncro);

        // WR: если "1" в данных, то отпускаем линию, слэйв прочитает "1"
        // RD: каждый входной бит у byte в "1", соответственно линию отпускаем
        if((*byte) & 0x01)
            pinRelease();

        // WR: следующий бит данных
        // RD: готовим очередной бит для приёма
        (*byte)>>=1; // (*x) = (*x) >> 1;

        deleyUs(Time10);

        // RD: до сюда должно быть меньше 15 мкс

        // читаем, что нам слэйв выставил, если мы пишем, то слэйв линию не трогает, она в "1"
        if (pin())
            (*byte) |=0x80;

        deleyUs(TimeSlot-(TimeSyncro + Time10));

        // если в данных был "0", то линия вернётся в исходное
        pinRelease();

        deleyUs(Time10);
    }

    _status = StatusPresence;
    return _status;
}





