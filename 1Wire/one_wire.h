#ifndef ONEWIRE_H
#define ONEWIRE_H

#include "mbed.h"

//-------------------------


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





class OneWire
{
public:
    OneWire(DigitalInOut apin);

    bool romCode(char *buff);

    // ROM function commands - Network layer
public:
    enum LineStatus {
        StatusPresence = 0,
        StatusShortCircuit,
        StatusAbsent,
    };


    /**
     * @brief reset
     * @return признак присутствия на шине
     */
    LineStatus reset();

    enum RomCommands {
        CommandReadRom = 0x33,
        CommandMatchRom = 0x55,
        CommandSearchRom = 0xF0,
        CommandSkipRom = 0xCC,
    };

    void readROM(); // 0x33 (или 0x0F - старая таблетка DS1990, без буквы А)
    void matchROM(); // 0x55
    void searchROM(); // 0xF0
    void skipROM(); // 0xCC

    bool isValid();

    enum Times {
        // из "Book of iButton Standards (AN937)"
        TimeSlot = 60,
        TimePdh = TimeSlot/2,
        TimePdl = 2*TimeSlot,
        TimeReset = 8*TimeSlot,
        // расчётное/подобранное
        TimeRelease = 2*TimeSlot/5, // 24 мкс
        TimePresence = 4*TimeSlot, // Верно ли?
        TimeSyncro = TimeSlot/6, // <15 мкс
    };

private:
    DigitalInOut _pin;
    bool _valid;
    unsigned char _romCode[8];

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

    inline void deleyUs(int us)
    {
        wait_us(us);
    }

    LineStatus readWriteByte(unsigned char *byte);
    unsigned char crc8(unsigned char data, unsigned char crc8val);

};

#endif // ONEWIRE_H
