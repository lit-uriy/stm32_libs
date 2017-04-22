#include "one_wire.h"

#include "1Wire/one_wire_device.h"

extern DigitalOut syncroPin;

extern bool test;

OneWire::OneWire(DigitalInOut apin)
    : _pin(apin)
    , _status(StatusUnknown)
    , _devices(16)
{
    syncroPin.write(0);

    // Line state: 1
    _pin.output();
    _pin.mode(OpenDrain); // Line state: 0
    pinRelease(); // Line state: 1
}

OneWire::LineStatus OneWire::status()
{
    return _status;
}

bool OneWire::romCode(unsigned char code[], char buff[])
{
    unsigned char i;

    for(i=0; i<8; i++){
        char cl = code[i] & 0x0F;
        char cm = code[i] >> 4;
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


void OneWire::addDevice(OneWireDevice *dev)
{
    _devices.append(dev);
    dev->wire = this;
}

void OneWire::removeDevice(OneWireDevice *dev)
{
    int index = _devices.indexOf(dev);
    if (!index)
        return;
    _devices.removeAt(index);
    dev->wire = 0;
}

YList<OneWireDevice *> OneWire::devices()
{
    return _devices;
}

OneWire::LineStatus OneWire::reset()
{
    // в начале на шине должна быть "1"
    if (!pin()){
        _status = StatusShortCircuit;
        return _status;
    }

    // выставляем "Reset pulse"
    pinLow();
    // выдержваем в течении TRSTL
    deleyUs(TimeReset);

    // снимаем "Reset pulse"
    pinRelease();
    // ждем возврата линии "0"->"1" (слэйв подождёт TimePdh, а затем выставит Presence на время TimePdl)
    deleyUs(TimeRelease); // здесь можно сделать измерение времени восстановления
    // на шине должна стать "1"
    if (!pin()){
        _status = StatusShortCircuit;
        return _status;
    }

    // подождем время, за которое слэйв гарантировано выставит "Presence pulse"
    deleyUs(TimeSlot);
    // проверим наличие "Presence pulse"
    if (pin()){
        _status = StatusAbsent;
        return _status;
    }

    //ждем завершения "Presence pulse" "0"->"1"
    deleyUs(TimePresence);
    // на шине должна стать "1"
    if (!pin()){
        _status = StatusShortCircuit;
        return _status;
    }

    deleyUs(TimeReset - TimePresence); // общая выдержка в отпущенном состоянии не менее TimeReset

    // нормальный "Presence pulse" был
    _status = StatusPresence;
    return _status;
}

OneWire::LineStatus OneWire::readWriteByte(unsigned char *byte)
{
    unsigned char j;

    for(j=0;j<8;j++)
    {
        // должна быть "1"
        if (!pin()) {
            printf("Error ocured on before syncro front\r\n");
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


unsigned char OneWire::crc8(unsigned char data, unsigned char crc8val)
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


// 0x33 (или 0x0F - старая таблетка DS1990, без буквы А)
bool OneWire::readROM(unsigned char aRomCode[])
{
    unsigned char temp = CommandReadRom;
    unsigned char i = 0;
    unsigned char crc = 0;

//    syncroPin.write(1);

    if (readWriteByte(&temp) != StatusPresence){
        printf("Error ocured on write comand \"Read ROM\"\r\n");
        return false; // что-то пошло не так, например, устройство отключили
    }

    test = true;

    syncroPin.write(1);
    for(i=0; i<8; i++){
        temp = 0xFF; // будем читать из слэйва
        if (readWriteByte(&temp) != StatusPresence){
            printf("Error ocured on read ROM cod\r\n");
            return false; // что-то пошло не так, например, устройство отключили
        }

        crc = crc8(temp, crc);
        aRomCode[i] = temp;
    }
    syncroPin.write(0);
    // проверяем CRC
    if (crc){
        printf("Error ocured on CRC check\r\n");
        return false; // CRC НЕ совпала
    }

    return true;
}

// 0x55
void OneWire::matchROM(const unsigned char aRomCode[])
{
    unsigned char temp = CommandMatchRom;
    int i;
    reset();
    if (readWriteByte(&temp) != StatusPresence){
        printf("Error ocured on write comand \"Match ROM\"\r\n");
        return; // что-то пошло не так, например, устройство отключили
    }
    for (i=0;i<8;i++) {
        temp = aRomCode[i];
        if (readWriteByte(&temp) != StatusPresence){
                printf("Error ocured on read answer on \"Match ROM\"\r\n");
                return; // что-то пошло не так, например, устройство отключили
        }
    }
}

// 0xF0
void OneWire::searchROM()
{

}

// 0xCC
void OneWire::skipROM()
{
    unsigned char temp = CommandSkipRom;
    reset();
    if (readWriteByte(&temp) != StatusPresence){
        printf("Error ocured on write comand \"Skip ROM\"\r\n");
        return; // что-то пошло не так, например, устройство отключили
    }
}

