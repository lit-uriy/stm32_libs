#ifndef NEW_ONEWIRE_H
#define NEW_ONEWIRE_H


#include "one_wire_rom_code.h"
#include "ylist.h"

class OneWireDevice;


class OneWire
{
public:
    enum LineStatus {
        StatusUnknown = 0,
        StatusPresence,
        StatusAlarming,
        StatusPulledUp,
        StatusAbsent = StatusPulledUp,
        StatusShortCircuit
    };

//    enum LineStatus {
//        // non-error statuses
//        StatusPresence = 0x01,
//        StatusPresenceMulty = 0x02,
//        StatusAlarming = 0x03,
//        StatusAbsent = 0x04,
//        // error statuses
//        StatusError = 0x81,
//        StatusShortCircuit = 0x82,
//        StatusUnknown = 0x83,
//    };


    enum RomCommands {
        CommandReadRom = 0x33,
        CommandMatchRom = 0x55,
        CommandSearchRom = 0xF0,
        CommandSkipRom = 0xCC,
    };

    enum ErrorCode {
        // младшая тетрада - "ЧТО?"
        ErrorNon = 0x00,
        ErrorBeforeSyncro,
        ErrorBeforePresence,
        ErrorAfterPresence,

        // старшая тетрада - "ГДЕ?"
        ErrorQueryReadRom = 0x10,
        ErrorAnswerReadRom = 0x20,
        ErrorCrcReadRom = 0x30,

        ErrorResetMatchRom = 0x40,
        ErrorQueryMatchRom = 0x50,
        ErrorAnswerMatchRom = 0x60,
        ErrorCrcMatchRom = 0x70,

        ErrorOnReset = 0x80,

        ErrorResetSkipRom = 0x90,
        ErrorQuerySkipRom = 0xA0,

        ErrorResetSearchRom = 0xB0,
        ErrorQuerySearchRom = 0xC0,
        ErrorAnswerSearchRom = 0xD0,
        ErrorCRCSearchRom = 0xE0,
    };


    OneWire();
    virtual ~OneWire(){}

    LineStatus status();
    int errorCode();

    // ***********************************************************
    // *          собирает информацию об устройствах             *
    // ***********************************************************
    OneWireRomCode findSingleDevice(); // по сути Read Rom


    // true - если ошибок небыло
    // false - если ошибка произошла, однако какое-то кол-во устройств могло быть найдено без ошибок
    bool findDevices(YList<OneWireRomCode*> *romCodes); // по сути Search Rom


    // ***********************************************************
    // *          связывает устройства спроволокой               *
    // ***********************************************************
    void addDevice(OneWireDevice *dev);
    void removeDevice(OneWireDevice *dev);
    YList<OneWireDevice *> devices();


    // ***********************************************************
    // *          Общие ROM-функции проволоки                    *
    // ***********************************************************

    // >>> Link Layer
    virtual LineStatus reset() = 0;

    virtual LineStatus readWriteBit(bool *bit) = 0;
    virtual LineStatus readWriteByte(unsigned char *byte) = 0;

    virtual void setStrongPullup(bool strong) = 0;
    // <<< Link Layer

    // >>> Network Layer

    bool readROM(OneWireRomCode *romCode);

    // Результаты поиска по ИЛИ
    enum SearchResult {
        SearchResultAbsent = 0,         // Rom-id не содержит новых данных
        SearchResultHasId = 0x01,       // Rom-id содержит новый id
        SearchResultHasNextId = 0x02,   // Есть ещё устройства
        SearchResultError = 0x04,       // Есть ошибка
    };
    // возвращает INT как результат операции ИЛИ для SearchResult
    int searchROM(OneWireRomCode *romCode, bool next = true);

    bool matchROM(const OneWireRomCode romCode);
    bool skipROM();
    // <<< Network Layer

    virtual void deleyMs(int ms) = 0;
    virtual void deleyUs(int us) = 0;


protected:
    LineStatus _status;
    int _errorCode;

private:
    OneWire(const OneWire &other);
    OneWire& operator=(const OneWire &other);

private:
    friend class OneWireDevice;

    YList<OneWireDevice *> _devices;

};

#endif // NEW_ONEWIRE_H
