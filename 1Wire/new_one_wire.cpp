#include "new_one_wire.h"

#include "one_wire_phy.h"

#include "one_wire_device.h"

#include "../utils/crc.h"


NewOneWire::NewOneWire()
    : _status(StatusUnknown)
    , _devices(16)
    , _errorCode(ErrorNon)
{

}

NewOneWire::LineStatus NewOneWire::status()
{
    return _status;
}

int NewOneWire::errorCode()
{
    return _errorCode;
}


OneWireRomCode NewOneWire::findSingleDevice()
{
    _errorCode = ErrorNon;
    OneWireRomCode rom;

    if (reset() != StatusPresence){
        _errorCode = ErrorOnReset | _errorCode;
        return OneWireRomCode(); // вернём пустышку
    }

    bool ok = readROM(&rom);
    if (ok)
        return rom;
    return OneWireRomCode(); // вернём пустышку
}

bool NewOneWire::findDevices(YList<OneWireRomCode*> *romCodes)
{
    printf("OneWire::findMultipleDevices()\n");
    int devCount = 0;
    OneWireRomCode romCode;

    while (1) {
        returnCounter = 0;
        bool next = bool(devCount);

//        printf("findMultipleDevices(); Init ROM=%s\r\n", romCode->romString());

        int result = searchROM(&romCode, next);

//        printf("\tsearchROM() = %d\n", result);

        if (result & SearchResultError) {
            printf("searchROM result=%02X, _status=%d\r\n", result, _status);
            return false; // но при этом в список уже могли попасть устройства
        }

        // есть очередной ROM-id
        if (result & SearchResultHasId) {
            // считаем CRC
            unsigned char crc = 0;
            for (int i = 0; i < 8; ++i) {
                crc = crc8(romCode.bytes[i], crc);
            }
            if (crc){
                printf("findMultipleDevices(); CRC ERROR: ROM=%s, CRC8=%02X\r\n", romCode.romString(), crc);
                _errorCode = ErrorCRCSearchRom | _errorCode;
                return false; // какая-то проблема на линии - надо начинать сначала
            }
            OneWireRomCode *r = new OneWireRomCode;
            *r = romCode;
            romCodes->append(r);// FIXME: Дубликаты не проверяются!
            devCount++;

            if (result & SearchResultHasNextId)
                continue;
            else
                break;

        }else {// SearchResultAbsent
            break;
        }
    }
    return true;
}

// FIXME: надо проверять сидит ли устройство на другой проволоке или нет
// если сидит, то сначала снять
void NewOneWire::addDevice(OneWireDevice *dev)
{
    _devices.append(dev);
    dev->_wire = this;
}

void NewOneWire::removeDevice(OneWireDevice *dev)
{
    int index = _devices.indexOf(dev);
    if (index < 0)
        return;
    _devices.removeAt(index);
    dev->_wire = 0;
}

YList<OneWireDevice *> NewOneWire::devices()
{
    return _devices;
}

bool NewOneWire::readROM(OneWireRomCode *romCode)
{
    unsigned char temp = CommandReadRom;
    unsigned char i = 0;
    unsigned char crc = 0;
    _errorCode = ErrorNon;

    if (reset() != StatusPresence){
        _errorCode = ErrorQueryReadRom | _errorCode;
        return false;
    }

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

int  NewOneWire::searchROM(OneWireRomCode *romCode, bool next)
{
    printf("OneWire::searchROM(next=%d)\n", next);
    _errorCode = ErrorNon;

    OneWireRomCode romA, romB, romC, romD;

    int testCounter = 0;

    static int lastConflictPos = 0;
    static bool done = false;

    if (!next){
        lastConflictPos = 0;
        done = false;
        returnCounter = 1;
    }

    int conflictPos = 0;
    bool present = false;


    // Ret = 0
    if (done){
        done = false;
        printf("searchROM done\r\n");
//        return present;
        returnCounter = 2;
        return SearchResultAbsent; // больше нет устройств
    }

    // 1 - The master begins the initialization sequence
    if (reset() != NewOneWire::StatusPresence){
        _errorCode = ErrorResetSearchRom | _errorCode;
        lastConflictPos = 0;
//        return present;
        returnCounter = 3;
        return SearchResultAbsent; // возможно на линии ни кого нет - надо начинать сначала
    }

    // 2 - The master will then issue the Search ROM command on the 1–Wire Bus
    unsigned char temp = CommandSearchRom;

    if (readWriteByte(&temp) != StatusPresence){
        _errorCode = ErrorQuerySearchRom | _errorCode;
        returnCounter = 4;
        return SearchResultAbsent; // что-то пошло не так, например, устройство отключили - надо начинать сначала
    }

    conflictPos = 0;
    for (int number = 1; number <= 64; ++number) {
        // 3 - The master reads one "bit[i]" from the 1–Wire bus.
        bool a = true; //  bit[i]
        bool b = true; // ~bit[i]
        bool c = true; // результирующий бит
        testCounter = 0;

        if (readWriteBit(&a) != StatusPresence){ // устройство выставит bit[i]
            _errorCode = ErrorAnswerSearchRom | _errorCode;
            returnCounter = 5;
            return SearchResultAbsent; // какая-то проблема на линии - надо начинать сначала
        }
        if (readWriteBit(&b) != StatusPresence){ // устройство выставит ~bit[i]
            _errorCode = ErrorAnswerSearchRom | _errorCode;
            returnCounter = 6;
            return SearchResultAbsent; // какая-то проблема на линии - надо начинать сначала
        }

        romA.setBit(number-1, a);
        romB.setBit(number-1, b);

        bool absent = a & b;
        bool conflict = !a & !b;

        if (absent){
            _status = StatusAbsent;
            lastConflictPos = 0;
            conflictPos = 0;
//        return present;
            return SearchResultAbsent;
            returnCounter = 7;
        }

        if (conflict){
            testCounter = 1;
            if (number == lastConflictPos){ // Добрались до предыдущего конфликта
                c = 1;
            }else if (number > lastConflictPos){ // ушли дальше предыдущего конфликта
                c = 0;
                conflictPos = number;
            }else {
                c = romCode->bit(number-1);
                romD.setBit(number-1, c);
                if (!c){ // НЕ добрались до предыдущего конфликта
                    conflictPos = number;
                }
            }
        }else{
            c = a;
        }

        romC.setBit(number-1, c);

        // 4 - The master now decides to write "bit[i]"
        romCode->setBit(number-1, c);
        if (readWriteBit(&c) != StatusPresence){
            _errorCode = ErrorAnswerSearchRom | _errorCode;
            returnCounter = 8;
            return SearchResultAbsent; // какая-то проблема на линии - надо начинать сначала
        }
//        printf("testCounter: %d\n", testCounter);

    } // for (number: 1 - 64)

    // 9 - Все биты ROM-кода известны и
    //     устройство готово для приёма команд транспортного уровня

    printf("Test ROM A: %s, conflictPos = %d, lastConflictPos = %d\n", romA.romString(), conflictPos, lastConflictPos);
    printf("Test ROM B: %s\n", romB.romString());
    printf("Test ROM C: %s\n", romC.romString());
    printf("Test ROM D: %s\n", romD.romString());

    lastConflictPos = conflictPos;
    if (!lastConflictPos){
        done = true;
        printf("searchROM Done\r\n");
        present = true;
        returnCounter = 9;
//        return present;
        return SearchResultHasId; // больше нет устройств - надо начинать сначала
    }

//        return present;
    returnCounter = 10;
    return SearchResultHasId |SearchResultHasNextId; // ещё есть устройства - можно продолжать
}

bool NewOneWire::skipROM()
{
    unsigned char temp = CommandSkipRom;
    _errorCode = ErrorNon;

    NewOneWire::LineStatus status = reset();
    if (status != NewOneWire::StatusPresence){
        _errorCode = ErrorResetSkipRom | _errorCode;
        return false;
    }
    if (readWriteByte(&temp) != StatusPresence){
        _errorCode = ErrorQuerySkipRom | _errorCode;
        return false; // что-то пошло не так, например, устройство отключили
    }
	return true;
}

bool NewOneWire::matchROM(const OneWireRomCode romCode)
{
    int i;
    _errorCode = ErrorNon;

//    syncroPin.write(1);

    NewOneWire::LineStatus status = reset();
    if (status != NewOneWire::StatusPresence){
        _errorCode = ErrorResetMatchRom | _errorCode;
//        syncroPin.write(0);
        return false;
    }

    unsigned char temp = NewOneWire::CommandMatchRom;
    if (readWriteByte(&temp) != NewOneWire::StatusPresence){
        _errorCode = ErrorQueryMatchRom | _errorCode;
//        syncroPin.write(0);
        return false; // что-то пошло не так, например, устройство отключили
    }
    for (i=0;i<8;i++) {
        temp = romCode.bytes[i];
        if (readWriteByte(&temp) != NewOneWire::StatusPresence){
            _errorCode = ErrorAnswerMatchRom | _errorCode;
//            syncroPin.write(0);
            return false; // что-то пошло не так, например, устройство отключили
        }
    }
//    syncroPin.write(0);
    return true;
}



NewOneWire::LineStatus NewOneWire::readWriteByte(unsigned char *byte)
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

NewOneWire::LineStatus NewOneWire::readWriteBit(bool *bit)
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





