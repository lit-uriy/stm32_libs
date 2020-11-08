
#include "mbed.h"
//#include "DS1820.h"

#include "ylist.h"
#include "one_wire.h"
#include "one_wire_device.h"
#include "y_ds1820.h"


#define DATA_PIN        A0
#define MAX_PROBES      16



DigitalIn mybutton(USER_BUTTON);
//bool mybutton = true;

DigitalOut syncroPin(NC);

DigitalOut greenLed(LED2);

DS1820* makeDevice(PinName name, int num_devices);

void convertTemperature(DS1820 *dev, int num_device);

void printTemperature(float temp, int num_devices);
void printRam(DS1820 *dev, int num_devices);

int main() {

    printf("\r\n----------------------------\r\n");

    /*********************************************
     *  Ищем все устройства которые откликнутся
     */
    if (mybutton) {
        printf("Button is not pressed, Finding unknown devices...\r\n");
        YList<OneWireRomCode*> roms;
        YList<Yds1820*> termometrs;

        DigitalInOut pin;
        OneWire wire(pin);

        bool ok = wire.findDevices(&roms);
        if (!ok){
            printf("Error ocured during the search; %s\r\n\n", wire.lastErrorText());
            if (roms.isEmpty()){
                exit(1);
            }
        }

        for(int i = 0; i < roms.size(); i++){
            unsigned char familyCode = roms.at(i)->familyCode();
            if ((familyCode == Yds1820::FamilyDs1820) || (familyCode == Yds1820::FamilyDs1822) || (familyCode == Yds1820::FamilyDs18B20)){
                termometrs.append(new Yds1820(roms, wire));
            }
        }

        printf("Found %d device(s)\n", roms.size());
        printf("\t%d of them are DS1820\n", termometrs.size());

        while(1) {
            convertTemperature(probe[0], 0);
            float temp = 0;
            for (int i = 0; i<num_devices; i++){
                float t = probe[i]->temperature();
                temp += t;
                printTemperature(t, i+1);
            }
            printf("Mean temperature: %3.1f %sC\r\n", temp/num_devices, (char*)(248));
            printf("\r\n");
            wait(1);
        }

    }else {// Button is pressed
        printf("Button is pressed, Working with predefined devices...\r\n");

        YList<DS1820*> termometrs;
        DigitalInOut pin;
        OneWire wire(pin);

        DS1820 *t1 = &DS1820("28FF0A1C661803D3", wire);
        termometrs << t1;
        DS1820 *t2 = &DS1820("28FFAEB36B180160");
        t2.setWire(wire);
        termometrs << t2;

        DS1990A ibutton("28FF6D3D6718017B");

        int count = termometrs.count();

        while(1){
            // вариант на ВСЕХ проволоках
            DS1820::convertTemperature();

            // вариант на КОНКРЕТНОЙ проволоке
            DS1820::convertTemperature(wire);

            for (int i=0; i < count; i++) {
                printTemperature(termometrs[i].temperature(), 1);
                printf("\r\n");
            }

            greenLed = ibutton.isPresent();

            wait(1);
        }
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
