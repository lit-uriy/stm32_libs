#include "y_ds1820.h"

Yds1820::Yds1820(unsigned char *aRomCode)
    : OneWireDevice(aRomCode)
{

}



int Yds1820::convertTemperature(bool wait, Devices device)
{
    return 111;
}

float Yds1820::temperature(char scale)
{
    return 21.4;
}

bool Yds1820::setResolution(unsigned int resolution)
{
    return false;
}
