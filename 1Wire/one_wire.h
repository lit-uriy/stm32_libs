#ifndef ONEWIRE_H
#define ONEWIRE_H

#include "mbed.h"

#include "ylist.h"

//-------------------------
//  ID термометра: 28 FF 55 9D 6F 14 04 3F
//


//    0x60=480us, 0x01=5us  0x01=8us
#define T15 0x02				//15us
#define Tpdh 0x07				//15...60us  0x07 (номинал 30us)
#define Tslot 0x0F			//60...120us 0x0F
#define Tpdl 0x1E				//60...240us 0x1E (120us)
#define Trstl 0x3C			//>=480us    0x3C

//-------------------------

#define T_SLOT	18			//длительность слота
#define T_10U	(T_SLOT/6)	//10мкс
#define T_RSTL	(8*T_SLOT)	//длительность сброса (reset)
#define T_RELS	(T_SLOT*2/5)//длительность востановления (release)
#define T_PRES	(4*T_SLOT)	//длительность отклика (presence)



#define delay_TM(del)	TMR2Delay(del)//задержка

#define TM_PRESNC		0x00	//TM присутствует
#define TM_SHRT_CRT		0x01	//Линия TM закорочена
#define TM_ABSNT		0x02	//TM отсутствует

#define TM_CMD_READROM		0x33	//Чтение ПЗУ (ID)
#define TM_CMD_MATCHROM		0x55	//Обращение по адресу
#define TM_CMD_SEARCHROM	0xF0	//поиск ПЗУ (ID)
#define TM_CMD_SKIPROM		0xCC	//пропустить ПЗУ (ID)

//-------------------------


class OneWireDevice;


class OneWireRomCode
{
public:
    OneWireRomCode()
    {
        for (int i = 0; i < 8; ++i) {
            bytes[i] = 0;
        }
    }

    // TODO: Нужен конструктор, который будет конструировать из строки.

    bool isNull()
    {
        for (int i = 0; i < 8; ++i) {
            if (bytes[i]) return false;
        }
        return true;
    }

    char* romString()
    {
        unsigned char i;

        for(i=0; i<8; i++){
            char cl = bytes[i] & 0x0F;
            char cm = bytes[i] >> 4;
            char resl = 0;
            char resm = 0;

            if (cm <= 9){ // числами
                resm = 0x30 + cm;
            }else{ // буквами
                resm = 55 + cm;
            }

            _romString[2*i] = resm;

            if (cl <= 9){ // числами
                resl = 0x30 + cl;
            }else{ // буквами
                resl = 0x37 + cl;
            }

            _romString[2*i+1] = resl;
        }
        _romString[8*2] = 0x00;
        return _romString;
    }
    unsigned char familyCode()
    {
        return bytes[0];
    }

    bool bit(int bitIndex)
    {
        int byteIndex = bitIndex/8;
        unsigned char byte = bytes[byteIndex];
        int bitInbyte = bitIndex%8;
        return byte & (1 << bitInbyte);
    }

    void setBit(int bitIndex, bool value)
    {
        int byteIndex = bitIndex/8;
        unsigned char byte = bytes[byteIndex];
        int bitInbyte = bitIndex%8;
        unsigned char mask = 1 << bitInbyte;
        bytes[byteIndex] = value? (byte | mask): (byte & ~mask);
    }
    unsigned char bytes[8];
    char _romString[2*8+1]; // в два раза больше символов + замыкающий нуль
};


/***************************************************
 *  1-Wire
 *
 */

class OneWire
{
public:
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


    OneWire(DigitalInOut apin);

    LineStatus status();
    int errorCode();

    // собирает информацию об устройствах
    OneWireRomCode findSingleDevice(); // по сути Read Rom
    LineStatus findMultipleDevices(YList<OneWireRomCode*> *romCodes); // по сути Search Rom

    bool findDevices(YList<OneWireRomCode*> *romList);

    void addDevice(OneWireDevice *dev);
    void removeDevice(OneWireDevice *dev);
    YList<OneWireDevice *> devices();


public:
    /**
     * @brief reset
     * @return признак присутствия на шине
     */
    LineStatus reset();

    bool readROM(OneWireRomCode *romCode);
    LineStatus searchROM(OneWireRomCode *romCode, bool next = true);
    bool matchROM(const OneWireRomCode romCode);
    bool skipROM();

private:
    enum Times {
        // из "Book of iButton Standards (AN937)"
        TimeSlot = 60,
        TimePdh = TimeSlot/2,
        TimePdl = 2*TimeSlot,
        TimeReset = 8*TimeSlot,
        // расчётное/подобранное
        TimeRelease = 2*TimeSlot/5, // 24 мкс, в документации написано 15 мкс - номинал
        TimePresence = 4*TimeSlot, // Верно ли?
        TimeSyncro = 2, // 3 мкс
        Time15 = 15,
        Time10 = 10
    };


    // Memory function commands - Transport layer
public:
    enum MemoryCommands {
        CommandReadRam = 0xF0,
        CommandReadEeprom = 0xA5,
        CommandReadSubkey = 0x66, // для старой таблетки DS1991
        CommandWriteScratchpad = 0x0F, // 0x96 - для старой таблетки DS1991
        CommandReadScratchpad = 0xAA, // 0x69 - для старой таблетки DS1991
        CommandCopyScratchpad = 0x55, // 0x3C - для старой таблетки DS1991
        CommandWriteSubkey = 0x99, // для старой таблетки DS1991
        CommandWritePassword = 0x5A, // для старой таблетки DS1991
        CommandWriteEeprom = 0x0F,
        CommandWriteStatus = 0x55,
        CommandReadStatus = 0xAA,
    };


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

    inline void deleyUs(int us)
    {
        wait_us(us);
    }

    inline void pinInit()
    {
        // Line state: 1
        _pin.output();
        _pin.mode(OpenDrain); // Line state: 0
        pinRelease(); // Line state: 1
    }

    LineStatus readWriteByte(unsigned char *byte);
    LineStatus readWriteBit(bool *bit);




private:
    friend class OneWireDevice;

    DigitalInOut _pin;
    LineStatus _status;

    YList<OneWireDevice *> _devices;

    int _errorCode;
};

#endif // ONEWIRE_H
