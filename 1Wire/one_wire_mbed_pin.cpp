#include "one_wire_mbed_pin.h"


OneWireMbedPin::OneWireMbedPin(DigitalInOut apin)
    : NewOneWire()
    , _pin(apin)
{
    pinInit();
}



NewOneWire::LineStatus OneWireMbedPin::reset()
{

    _errorCode = ErrorNon;
    // в начале на шине должна быть "1"
//    syncroPin.write(1);
    if (!pin()){
        _status = StatusShortCircuit;
        _errorCode = ErrorBeforeSyncro;
//        syncroPin.write(0);
        return _status;
    }

    // выставляем "Reset pulse"
    pinLow();
    // выдержваем в течении TRSTL
    deleyUs(TimeReset);

    // снимаем "Reset pulse"
    pinRelease();
    // ждем возврата линии "0"->"1" (слэйв подождёт TimePdh, а затем выставит Presence на время TimePdl)
    deleyUs(Time10); // здесь можно сделать измерение времени восстановления
    // на шине должна стать "1"
    if (!pin()){
        _status = StatusShortCircuit;
        _errorCode = ErrorBeforePresence;
//        syncroPin.write(0);
        return _status;
    }

    // подождем время, за которое слэйв гарантировано выставит "Presence pulse"
    deleyUs(TimeSlot);
    // проверим наличие "Presence pulse"
    if (pin()){
        _status = StatusAbsent;
//        syncroPin.write(0);
        return _status;
    }

    //ждем завершения "Presence pulse" "0"->"1"
    deleyUs(TimePresence);
    // на шине должна стать "1"
    if (!pin()){
        _status = StatusShortCircuit;
        _errorCode = ErrorAfterPresence;
//        syncroPin.write(0);
        return _status;
    }

    deleyUs(TimeReset - TimePresence); // общая выдержка в отпущенном состоянии не менее TimeReset

    // нормальный "Presence pulse" был
    _status = StatusPresence;
//    syncroPin.write(0);
    return _status;
}

NewOneWire::LineStatus OneWireMbedPin::readWriteBit(bool *bit)
{
    _errorCode = ErrorNon;

    // должна быть "1"
    if (!pin()) {
        _errorCode = ErrorBeforeSyncro;
        _status = StatusShortCircuit;
        return _status;
    }
    //выставляем "Синхрофронт" на 10мкс, т.к. через 15 мкс слэйв будет читать данные

    pinLow();
    deleyUs(TimeSyncro);

    // WR: если "1" в данных, то отпускаем линию, слэйв прочитает "1"
    // RD: каждый входной бит у byte в "1", соответственно линию отпускаем
    if(*bit)
        pinRelease();

    deleyUs(Time10);

    // RD: до сюда должно быть меньше 15 мкс

    // читаем, что нам слэйв выставил, если мы пишем, то слэйв линию не трогает, она в "1"
    *bit = pin();

    deleyUs(TimeSlot-(TimeSyncro + Time10));

    // если в данных был "0", то линия вернётся в исходное
    pinRelease();

    deleyUs(Time10);

    _status = StatusPresence;
    return _status;

}

NewOneWire::LineStatus OneWireMbedPin::readWriteByte(unsigned char *byte)
{

    unsigned char j;
    _errorCode = ErrorNon;

    for(j=0;j<8;j++)
    {
        // должна быть "1"
        if (!pin()) {
            _errorCode = ErrorBeforeSyncro;
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

void OneWireMbedPin::setStrongPullup(bool strong)
{
    if (strong){
        _pin.mode(PushPullNoPull);
        _pin.write(1);
    }else{
        pinInit();
    }
}

void OneWireMbedPin::deleyMs(int ms)
{
    wait_ms(ms);
}

void OneWireMbedPin::deleyUs(int us)
{
    wait_us(us);
}
