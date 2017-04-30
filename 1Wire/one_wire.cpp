#include "one_wire.h"

#include "one_wire_device.h"
#include "../utils/crc.h"

extern DigitalOut syncroPin;

extern bool test;

OneWire::OneWire(DigitalInOut apin)
    : _pin(apin)
    , _status(StatusUnknown)
    , _devices(16)
{
    pinInit();
    syncroPin.write(0);
}

OneWire::LineStatus OneWire::status()
{
    return _status;
}

int OneWire::errorCode()
{
    return _errorCode;
}


OneWireRomCode OneWire::findSingleDevice()
{
    _errorCode = ErrorNon;
    OneWireRomCode rom;
    OneWire::LineStatus status = reset();
    if (status != OneWire::StatusPresence){
        _errorCode = ErrorOnReset | _errorCode;
        return OneWireRomCode(); // вернём пустышку
    }

    bool ok = readROM(&rom);
    if (ok)
        return rom;
    return OneWireRomCode(); // вернём пустышку
}

// FIXME: Не реализовано
YList<OneWireRomCode> OneWire::findMultipleDevices()
{
    YList<OneWireRomCode> roms;

    return roms; // вернём пустышку
}

// FIXME: надо проверять сидит ли устройство на другой проволоке или нет
// если сидит, то сначала снять
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
bool OneWire::readROM(OneWireRomCode *romCode)
{
    unsigned char temp = CommandReadRom;
    unsigned char i = 0;
    unsigned char crc = 0;
    _errorCode = ErrorNon;

    if (readWriteByte(&temp) != StatusPresence){
        _errorCode = ErrorQueryReadRom | _errorCode;
        return false; // что-то пошло не так, например, устройство отключили
    }

    for(i=0; i<8; i++){
        temp = 0xFF; // будем читать из слэйва
        if (readWriteByte(&temp) != StatusPresence){
            _errorCode = ErrorAnswerReadRom | _errorCode;
            return false; // что-то пошло не так, например, устройство отключили
        }

        crc = crc8(temp, crc);
        romCode->bytes[i] = temp;
    }

    // проверяем CRC
    if (crc){
        _errorCode = ErrorCrcReadRom | _errorCode;
        return false; // CRC НЕ совпала
    }

    return true;
}

void OneWire::searchROM()
{
    // FIXME: не реализовано
}

bool OneWire::skipROM()
{
    unsigned char temp = CommandSkipRom;
    _errorCode = ErrorNon;

    OneWire::LineStatus status = reset();
    if (status != OneWire::StatusPresence){
        _errorCode = ErrorResetSkipRom | _errorCode;
        return false;
    }
    if (readWriteByte(&temp) != StatusPresence){
        _errorCode = ErrorQuerySkipRom | _errorCode;
        return false; // что-то пошло не так, например, устройство отключили
    }
	return true;
}

bool OneWire::matchROM(const OneWireRomCode romCode)
{
    int i;
    _errorCode = ErrorNon;

//    syncroPin.write(1);

    OneWire::LineStatus status = reset();
    if (status != OneWire::StatusPresence){
        _errorCode = ErrorResetMatchRom | _errorCode;
//        syncroPin.write(0);
        return false;
    }

    unsigned char temp = OneWire::CommandMatchRom;
    if (readWriteByte(&temp) != OneWire::StatusPresence){
        _errorCode = ErrorQueryMatchRom | _errorCode;
//        syncroPin.write(0);
        return false; // что-то пошло не так, например, устройство отключили
    }
    for (i=0;i<8;i++) {
        temp = romCode.bytes[i];
        if (readWriteByte(&temp) != OneWire::StatusPresence){
            _errorCode = ErrorAnswerMatchRom | _errorCode;
//            syncroPin.write(0);
            return false; // что-то пошло не так, например, устройство отключили
        }
    }
//    syncroPin.write(0);
    return true;
}



OneWire::LineStatus OneWire::readWriteByte(unsigned char *byte)
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

OneWire::LineStatus OneWire::readWriteBit(bool *bit)
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





