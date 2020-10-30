
#include "mbed.h"
#include "DS1820.h"



#define DATA_PIN        A0
#define MAX_PROBES      16



DigitalIn mybutton(USER_BUTTON);
//bool mybutton = true;

DigitalOut syncroPin(NC);
//bool test = false;

DS1820* makeDevice(PinName name, int num_devices);

void convertTemperature()
{

}

void convertTemperature(DS1820 *dev, int num_device);

void printTemperature(float temp, int num_devices);
void printRam(DS1820 *dev, int num_devices);

int main()
{
    printf("---------------- Test IO pin ----------\r\n");
    DigitalOut led(LED2);

    DigitalInOut data_pin(DATA_PIN);
    // INIT
    // -- ONEWIRE_INIT
    data_pin.output();
    data_pin.mode(OpenDrain); // OpenDrain = OpenDrainPullUp !!!

    bool state = mybutton.read();

    while(1){
        if (state) {// Button is NOT pressed
            data_pin.write(1);
            led.write(1);
            printf("Write 1\r\n");
        }else {// Button is pressed
            data_pin.write(0);
            led.write(0);
            printf("Write 0\r\n");
        }
        while(state == mybutton.read()){}
        state = mybutton.read();
    }
}


DS1820* makeDevice(PinName name, int num_devices)
{
    char romString[2*8+1]; // в два раза больше символов + замыкающий нуль

    DS1820 *dev = new DS1820(name);
    dev->romCode(romString);
    printf("Found %d device, ROM=%s\r\n", num_devices, romString);
    printf("\tparasite powered: %s\r\n", dev->isParasitePowered()? "Yes": "No");
    char ramString[2*9+1]; // в два раза больше символов + замыкающий нуль
    dev->ramToHex(ramString);
    printf("\tRAM: %s\r\n", ramString);
    printf("\tresolution: %d bits\r\n", dev->resolution());
    printf("\r\n");

    return dev;
}

void convertTemperature(DS1820 *dev, int num_device)
{
    dev->convertTemperature(true, DS1820::all_devices);         //Start temperature conversion, wait until ready
}

void printTemperature(float temp, int num_devices)
{
    printf("Device %d returns %3.1f %sC\r\n", num_devices, temp, (char*)(248));
}


void printRam(DS1820 *dev, int num_devices)
{
    char ramString[2*9+1]; // в два раза больше символов + замыкающий нуль
    dev->ramToHex(ramString);
    printf("Device %d RAM: %s\r\n", num_devices, ramString);
}
