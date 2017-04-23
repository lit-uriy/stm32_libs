#include "one_wire_device.h"

OneWireDevice::OneWireDevice(unsigned char *aRomCode)
    : _romCode(aRomCode)
    , _wire(0)
{

}

unsigned char OneWireDevice::familyCode()
{
    return _romCode[0];
}

// 0x33 (или 0x0F - старая таблетка DS1990, без буквы А)
bool OneWireDevice::readROM()
{
    if (!_wire)
        return false;
    //--
    unsigned char temp = CommandReadRom;
    unsigned char i = 0;
    unsigned char crc = 0;

    if (_wire->readWriteByte(&temp) != OneWire::StatusPresence){
        printf("Error ocured on write comand \"Read ROM\"\r\n");
        return false; // что-то пошло не так, например, устройство отключили
    }

    for(i=0; i<8; i++){
        temp = 0xFF; // будем читать из слэйва
        if (_wire->readWriteByte(&temp) != OneWire::StatusPresence){
            printf("Error ocured on read ROM cod\r\n");
            return false; // что-то пошло не так, например, устройство отключили
        }

        crc = crc8(temp, crc);
        _romCode[i] = temp;
    }

    // проверяем CRC
    if (crc){
        printf("Error ocured on readROM() CRC check\r\n");
        return false; // CRC НЕ совпала
    }

    return true;
}

bool OneWireDevice::matchROM()
{
    if (!_wire)
        return false;
    //--
    unsigned char temp = CommandMatchRom;
    int i;
    _wire->reset();
    if (_wire->readWriteByte(&temp) != OneWire::StatusPresence){
        printf("Error ocured on write comand \"Match ROM\"\r\n");
        return false; // что-то пошло не так, например, устройство отключили
    }
    for (i=0;i<8;i++) {
        temp = _romCode[i];
        if (_wire->readWriteByte(&temp) != OneWire::StatusPresence){
            printf("Error ocured on read answer on \"Match ROM\"\r\n");
            return false; // что-то пошло не так, например, устройство отключили
        }
    }
    return true;
}

void OneWireDevice::searchROM()
{
    if (!_wire)
        return;
    
}

void OneWireDevice::skipROM()
{
    if (!_wire)
        return;
    //--
    unsigned char temp = CommandSkipRom;
    _wire->reset();
    if (_wire->readWriteByte(&temp) != OneWire::StatusPresence){
        printf("Error ocured on write comand \"Skip ROM\"\r\n");
        return; // что-то пошло не так, например, устройство отключили
    }
}

bool OneWireDevice::romString(char buff[])
{
    unsigned char i;

    for(i=0; i<8; i++){
        char cl = _romCode[i] & 0x0F;
        char cm = _romCode[i] >> 4;
        char resl = 0;
        char resm = 0;

        if (cm <= 9){ // числами
            resm = 0x30 + cm;
        }else{ // буквами
            resm = 55 + cm;
        }

        buff[2*i] = resm;

        if (cl <= 9){ // числами
            resl = 0x30 + cl;
        }else{ // буквами
            resl = 0x37 + cl;
        }

        buff[2*i+1] = resl;
    }
    buff[8*2] = 0x00;
    return true;
}


unsigned char OneWireDevice::crc8(unsigned char data, unsigned char crc8val)
{
    unsigned char cnt;
    cnt=8;
    do{
        if ((data^crc8val)&0x01){
            crc8val^=0x18; //0x18-Полином:0b00011000=>X^8+X^5+X^4+1
            crc8val>>=1;
            crc8val|=0x80;}
        else {
            crc8val>>=1;
        }
        data>>=1;
    }while (--cnt);
    return crc8val;
}

