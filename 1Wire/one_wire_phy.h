#ifndef ONEWIRE_PHY_H
#define ONEWIRE_PHY_H

#include "mbed.h"


//-------------------------


/***************************************************
 *  1-Wire
 *
 ***************************************************/

class OneWirePhy
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


    enum ErrorCode {
        // младшая тетрада - "ЧТО?"
        ErrorNon = 0x00,
        ErrorBeforeSyncro,
        ErrorBeforePresence,
        ErrorAfterPresence,
    };


    OneWirePhy(DigitalInOut apin);

    LineStatus status();
    int errorCode();



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


    // >>> Physical Layer
    inline void pinInit()
    {
        // Line state: 1
        _pin.output();
        _pin.mode(OpenDrain); // Line state: 0
        pinRelease(); // Line state: 1
    }

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


    inline void deleyUs(int us)
    {
        wait_us(us);
    }


private:
    int _errorCode;
};

#endif // ONEWIRE_PHY_H
