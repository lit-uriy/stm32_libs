#include "y_ds1820.h"

#include "../utils/crc.h"

/*
 *
 * Error ocured on before syncro front
 * Error ocured on write comand "Match ROM"
 * Device 28FF559D6F14043F returns -1000.0 C
 *
 *
 * сначала в ОЗУ почему-то по нулям, кроме конфигурации:
 * 00 00 00 00 08 00 00 00
 *
 * а потом, после запуска преобразования так:
 * 9A 01 55 00 7F FF 7F 10
 */

Yds1820::Yds1820(OneWireRomCode aRomCode, OneWire *awire)
    : OneWireDevice(aRomCode, awire)
    , _parasitePower(true)
{
    for (int i = 0; i < 9; ++i) {
        _ram.byte[i] = 0;
    }
}

bool Yds1820::readPowerSupply(Devices device)
{
    if (!matchROM()){
        return false;
    }

    unsigned char temp = CommandReadPowerSupply;
    if (wire()->readWriteByte(&temp) != OneWire::StatusPresence){
        printf("Error ocured on write comand \"Read Power Supply\"\r\n");
        return false; // что-то пошло не так, например, устройство отключили
    }
    _parasitePower = !wire()->pin();
    return _parasitePower;
}

bool Yds1820::ramString(char buff[])
{
    unsigned char i;
    unsigned char maxByte = 9;

    for(i=0; i<maxByte; i++){
        char cl = _ram.byte[i] & 0x0F;
        char cm = _ram.byte[i] >> 4;
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
    buff[maxByte*2] = 0x00;
    return true;
}

// статическая, для всех термометров на проволоке
int Yds1820::convertTemperature(OneWire *awire)
{
    // Convert temperature into scratchpad RAM for all devices at once
    int delay_time = 750; // Default delay time

    awire->skipROM();
    if (awire->status() != OneWire::StatusPresence)
        return -1;
    // собственно запуск преобразования
    unsigned char temp = CommandConvertT;
    if (awire->readWriteByte(&temp) != OneWire::StatusPresence){
        printf("Error ocured on write comand \"Convert T\", status: %d\r\n", awire->status());
        return -1; // что-то пошло не так, например, устройство отключили
    }

    // считаем что все устройства с паразитным питанием
    awire->setStrongPullup(true);
    wait_ms(delay_time);
    awire->setStrongPullup(false);
    delay_time = 0;

    return delay_time;
}


// индивидуальная функция экземпляра / устройства
int Yds1820::convertTemperature()
{
    // Convert temperature into scratchpad RAM for all devices at once
    int delay_time = 750; // Default delay time
    char resolution;


    if (matchROM())
        return -1; // что-то пошло не так, например, устройство отключили

    resolution = _ram.config & 0x60;
    if (resolution == 0x00) // 9 bits
        delay_time = 94;
    if (resolution == 0x20) // 10 bits
        delay_time = 188;
    if (resolution == 0x40) // 11 bits. Note 12bits uses the 750ms default
        delay_time = 375;

    // собственно запуск преобразования
    unsigned char temp = CommandConvertT;
    if (wire()->readWriteByte(&temp) != OneWire::StatusPresence){
        printf("Error ocured on write comand \"Convert T\", status: %d\r\n", wire()->status());
        return -1; // что-то пошло не так, например, устройство отключили
    }


    if (_parasitePower)
        wire()->setStrongPullup(true);

    wait_ms(delay_time);

    if (_parasitePower)
        wire()->setStrongPullup(false);

    wait_ms(delay_time);
    delay_time = 0;

    return delay_time;
}

float Yds1820::temperature(char scale)
{
    // The data specs state that count_per_degree should be 0x10 (16), I found my devices
    // to have a count_per_degree of 0x4B (75). With the standard resolution of 1/2 deg C
    // this allowed an expanded resolution of 1/150th of a deg C. I wouldn't rely on this
    // being super acurate, but it does allow for a smooth display in the 1/10ths of a
    // deg C or F scales.
    float answer, remaining_count, count_per_degree;
    int reading, reading2;
    if (!readRam())
        // Indicate we got a CRC error
        answer = invalid_conversion;
    else {
        reading = (_ram.byte[1] << 8) + _ram.byte[0];
        reading2 = _ram.currentTemp;
        if (reading & 0x8000) { // negative degrees C
            reading = 0-((reading ^ 0xffff) + 1); // 2's comp then convert to signed int
        }
        answer = reading +0.0; // convert to floating point
        if ((familyCode() == FamilyDs18B20 ) || (familyCode() == FamilyDs1822)) {
            answer = answer / 16.0f;
        } else {
            remaining_count = _ram.byte[6];
            count_per_degree = _ram.byte[7];
            answer = floor(answer/2.0f) - 0.25f + (count_per_degree - remaining_count) / count_per_degree;
        }
        if (scale=='F' or scale=='f')
            // Convert to deg F
            answer = answer * 9.0f / 5.0f + 32.0f;
    }
    return answer;
}

bool Yds1820::setResolution(unsigned int resolution)
{
    return false;
}

bool Yds1820::readRam()
{
    if (!matchROM()){
        return false;
    }
    unsigned char temp = CommandReadScratchpad;
    if (wire()->readWriteByte(&temp) != OneWire::StatusPresence){
        printf("Error ocured on write comand \"Read Scratchpad\"\r\n");
        return false; // что-то пошло не так, например, устройство отключили
    }

    unsigned char crc = 0;
    for (int i = 0; i < 9; ++i) {
        temp = 0xFF;
        if (wire()->readWriteByte(&temp) != OneWire::StatusPresence){
            printf("Error ocured on read scratchpad\r\n");
            return false; // что-то пошло не так, например, устройство отключили
        }
        _ram.byte[i] = temp;
        crc = crc8(temp, crc);
    }
    // проверяем CRC
    if (crc){
        printf("Error ocured on read scratchpad CRC check\r\n");
        return false; // CRC НЕ совпала
    }

    return true;

}
