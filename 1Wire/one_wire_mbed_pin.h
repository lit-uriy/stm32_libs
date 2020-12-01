#ifndef ONEWIRE_MBEDPIN_H
#define ONEWIRE_MBEDPIN_H

#include "mbed.h"
#include "new_one_wire.h"

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


/***************************************************
 *  1-Wire
 *
 */

class OneWireMbedPin: public NewOneWire
{
public:

    OneWireMbedPin(DigitalInOut apin);
    virtual ~OneWireMbedPin(){}

public:
    /**
     * @brief reset
     * @return признак присутствия на шине
     */
    virtual LineStatus reset();

    virtual LineStatus readWriteBit(bool *bit);
    virtual LineStatus readWriteByte(unsigned char *byte);

    virtual void setStrongPullup(bool strong);

    virtual void deleyMs(int ms);
    virtual void deleyUs(int us);

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


    inline void pinInit()
    {
        // Line state: 1
        _pin.output();
        _pin.mode(OpenDrain); // Line state: 0
        pinRelease(); // Line state transit to 1
        while(!pin()){} // wait 1
        wait_us(TimePresence);
    }





private:
    friend class OneWireDevice;

    DigitalInOut _pin;
};

#endif // ONEWIRE_MBEDPIN_H
