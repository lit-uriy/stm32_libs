#include "y_ds1820.h"

Yds1820::Yds1820(unsigned char *aRomCode)
    : OneWireDevice(aRomCode)
{

}



int Yds1820::convertTemperature(bool wait, Devices device)
{
    // Convert temperature into scratchpad RAM for all devices at once
    int delay_time = 750; // Default delay time
    char resolution;
    bool parasetPower = true;



    unsigned char temp = CommandConvertT;
    if (readWriteByte(&temp) != OneWire::StatusPresence){
        printf("Error ocured on write comand \"Convert T\"\r\n");
        return -1; // что-то пошло не так, например, устройство отключили
    }
    if (parasetPower){
        // TODO: тут надо выставить жёсткую "1" на линию
        wait_ms(delay_time);
    }

    return 0;
}

float Yds1820::temperature(char scale)
{
    return 21.4;
}

bool Yds1820::setResolution(unsigned int resolution)
{
    return false;
}
