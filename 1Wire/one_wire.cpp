#include "one_wire.h"

OneWire::OneWire(DigitalInOut apin)
    : _pin(apin)
    , _valid(false)
{
    // Line state: 1
    _pin.output();
    _pin.mode(OpenDrain); // Line state: 0
    pinRelease(); // Line state: 1
}

bool OneWire::romCode(char *buff)
{
    unsigned char i;

    for(i=0; i<8; i++){
        char cl = _romCode[i] & 0x0F;
        char cm = _romCode[i] >> 4;
		char resl = 0;
		char resm = 0;

        if (cm >= 0 & cm <= 9){ // числами
            resm = 0x30 + cm;
        }else{ // буквами
            resm = 55 + cm;
        }
		
		buff[2*i] = resm;

        if (cl >= 0 & cl <= 9){ // числами
            resl = 0x30 + cl;
        }else{ // буквами
            resl = 0x37 + cl;
        }
		
		buff[2*i+1] = resl;
    }
	
    return _valid;
}

OneWire::LineStatus OneWire::reset()
{
    // в начале на шине должна быть "1"
    if (!pin())
        return StatusShortCircuit;

    // выставляем "Reset pulse"
    pinLow();
    // выдержваем в течении TRSTL
    deleyUs(TimeReset);

    // снимаем "Reset pulse"
    pinRelease();
    // ждем возврата линии "0"->"1" (слэйв подождёт TimePdh, а затем выставит Presence на время TimePdl)
    deleyUs(TimeRelease); // здесь можно сделать измерение времени восстановления
    // на шине должна стать "1"
    if (!pin())
        return StatusShortCircuit;

    // подождем время, за которое слэйв гарантировано выставит "Presence pulse"
    deleyUs(TimeSlot);
    // проверим наличие "Presence pulse"
    if (pin())
        return StatusAbsent;

    //ждем завершения "Presence pulse" "0"->"1"
    deleyUs(TimePresence);
    // на шине должна стать "1"
    if (!pin())
        return StatusShortCircuit;

    // нормальный "Presence pulse" был
    return StatusPresence;
}

OneWire::LineStatus OneWire::readWriteByte(unsigned char *byte)
{
    unsigned char j;

    for(j=0;j<8;j++)
    {
        // должна быть "1"
        if (!pin())
            return StatusShortCircuit;
        //выставляем "Синхрофронт" на 10мкс, т.к. через 15 мкс слэйв будет читать данные
        pinLow();
        deleyUs(TimeSyncro);

        // если "1" в данных, то отпускаем линию, слэйв прочитает "1"
        if((*byte) & 0x01)
            pinRelease();

        // следующий бит данных
        (*byte)>>=1; // (*x) = (*x) >> 1;

        deleyUs(TimeSyncro);

        // читаем, что нам слэйв выставил, если мы пишем, то слэйв линию не трогает, она в "1"
        if (pin())
            (*byte) |=0x80;

        deleyUs(TimeSlot-(2*TimeSyncro));

        // если в данных был "0", то линия вернётся в исходное
        pinRelease();

        deleyUs(2*TimeSyncro);
    }
    return StatusPresence;
}


unsigned char OneWire::crc8(unsigned char data, unsigned char crc8val)
{
unsigned char cnt;
cnt=8;
    do{
        if ((data^crc8val)&0x01){
            crc8val^=0x18; //0x18-Полином:0b00011000=>X^8+X^5+X^4+1
            crc8val>>=1;
            crc8val|=0x80;}
        else {crc8val>>=1;}
        data>>=1;
    }while (--cnt);
    return crc8val;
}


// 0x33 (или 0x0F - старая таблетка DS1990, без буквы А)
void OneWire::readROM()
{
    unsigned char temp = CommandReadRom;
    unsigned char i = 0;
    unsigned char crc = 0;

    if (readWriteByte(&temp) != StatusPresence)
        return; // что-то пошло не так, например, устройство отключили

    temp = 0xFF; // будем читать из слэйва
    for(i=0; i<8; i++)
    {
        if (readWriteByte(&temp) != StatusPresence)
            return; // что-то пошло не так, например, устройство отключили

        crc = crc8(temp, crc);
        _romCode[i] = temp;
    }
    // проверяем CRC
    if (crc)
        _valid = false; // CRC не совпала

    _valid = true;
}

// 0x55
void OneWire::matchROM()
{

}

// 0xF0
void OneWire::searchROM()
{

}

// 0xCC
void OneWire::skipROM()
{

}

