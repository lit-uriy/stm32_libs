#ifndef ONEWIRE_H
#define ONEWIRE_H


#include "one_wire_rom_code.h"
#include "ylist.h"

class OneWireDevice;

template <class T>
class OneWirePhy;

/***************************************************
 *  1-Wire
 *
 */

class NewOneWire
{
public:
    enum NewLineStatus {
        NewStatusUnknown = 0,
        NewStatusPulledUp,
        NewStatusShortCircuit,
        NewStatusNormal = NewStatusPulledUp
    };

    enum LineStatus {
        // non-error statuses
        StatusPresence = 0x01,
        StatusPresenceMulty = 0x02,
        StatusAlarming = 0x03,
        StatusAbsent = 0x04,
        // error statuses
        StatusError = 0x81,
        StatusShortCircuit = 0x82,
        StatusUnknown = 0x83,
    };


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


    NewOneWire(DigitalInOut apin);

    LineStatus status();
    int errorCode();

    // ***********************************************************
    // *          собирает информацию об устройствах             *
    // ***********************************************************
    OneWireRomCode findSingleDevice(); // по сути Read Rom
    LineStatus findMultipleDevices(YList<OneWireRomCode*> *romCodes); // по сути Search Rom
    bool findDevices(YList<OneWireRomCode*> *romList);


    // ***********************************************************
    // *          связывает устройства спроволокой               *
    // ***********************************************************
    void addDevice(OneWireDevice *dev);
    void removeDevice(OneWireDevice *dev);
    YList<OneWireDevice *> devices();


public:
    // ***********************************************************
    // *          Общие ROM-функции проволоки                    *
    // ***********************************************************

    // >>> Physical Layer
    inline bool pin()
    {
        return _pin.read();
    }

    inline void pinLow()
    {
        _pin.write(0);
    }

    inline void pinRelease()
    {
        _pin.write(1);
    }


    inline void setStrongPullup(bool strong)
    {
        if (strong){
            _pin.mode(PushPullNoPull);
            _pin.write(1);
        }else{
            pinInit();
        }
    }

    inline void pinInit()
    {
        // Line state: 1
        _pin.output();
        _pin.mode(OpenDrain); // Line state: 0
        pinRelease(); // Line state: 1
    }

    /**
     * @brief reset
     * @return признак присутствия на шине
     */
    LineStatus reset();

    LineStatus readWriteBit(bool *bit);

    // <<< Physical Layer

    // >>> Link Layer
    LineStatus readWriteByte(unsigned char *byte);
    // <<< Link Layer

    // >>> Network Layer
    bool readROM(OneWireRomCode *romCode);
    LineStatus searchROM(OneWireRomCode *romCode, bool next = true);
    bool matchROM(const OneWireRomCode romCode);
    bool skipROM();
    // <<< Network Layer





    inline void deleyUs(int us)
    {
        wait_us(us);
    }





private:
    friend class OneWireDevice;
    OneWirePhy _phy;
    LineStatus _status;

    YList<OneWireDevice *> _devices;

    int _errorCode;
};

#endif // ONEWIRE_H
