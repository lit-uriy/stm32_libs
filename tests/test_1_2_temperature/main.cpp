

#include "ylist.h"
#include "new_one_wire.h"
#include "one_wire_mbed_pin.h"
#include "y_ds1820.h"
#include "mbed.h"



#define DATA_PIN        A0
#define MAX_PROBES      16


int main()
{
    printf("===============================================\r\n");
    printf("               Test-1.2                        \r\n");
    printf("Build with mBed %d.%d.%d\r\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
    printf("===============================================\r\n");
    printf("\r\n");

    // ***** Реализация физического (Physical) уровня и звена данных (Link) 1-Wire

    // -- программная, через ножку контроллера
#if (1)
    DigitalInOut pin(DATA_PIN); // Mbed-ножка контроллера
    OneWireMbedPin *oneWireConcrete = new OneWireMbedPin(pin);
#else
    // -- аппаратная, через USART
    Serial port(USBTX, USBRX); // Mbed-USART
    OneWireMbedSerial *oneWireConcrete = new OneWireMbedSerial(port);
#endif

    // ***** Собственно проволока 1-Wire
    NewOneWire *wire = oneWireConcrete;

    // список с ROM-кодами
    YList<OneWireRomCode*> roms;

    bool ok = wire->findDevices(&roms);
    if (!ok){
        printf("Error ocured during the search 1-Wire devices; ErrorCode: %d\r\n", wire->errorCode());
    }

    if (roms.isEmpty()){
        printf("No devices found!\r\n");
        while(1){}
    }

    YList<Yds1820*> list;

    printf("Found %d device(s):\r\n", roms.size());
    for(int i = 0; i < roms.size(); i++){
        OneWireRomCode *rom = roms.at(i);

        // Создадим термодатчики
        unsigned char familyCode = rom->familyCode();
        if ((familyCode == Yds1820::FamilyDs1820) || (familyCode == Yds1820::FamilyDs1822) || (familyCode == Yds1820::FamilyDs18B20)){
            Yds1820 *dev = new Yds1820(*(roms.at(i)), wire);
            printf("Device #%d, ROM=%s\r\n", i, rom->romString());
            printf("\tparasite powered: %s\r\n", dev->isParasitePowered()? "Yes": "No");
            printf("\tresolution: %d bits\r\n", dev->resolution());
            printf("\r\n");
            list.append(dev);
        }
    }


    while (1) {
        int delay = Yds1820::convertTemperature(wire);
        wait_ms(delay);

        float maxT = -100;
        float minT = 100;
        float meanT = 0;
        float temp = 0;

        for (int i = 0; i < list.size(); ++i) {
            float t = list.at(i)->temperature();
            temp += t;
            if (t < minT)
                  minT = t;
            if (t > maxT)
                  maxT = t;

            printf("Device #%d returns %3.1f C\r\n", i, t);
        }
        meanT = temp/list.size();
        printf("Temperature: Min:%3.1f | Mean:%3.1f | Max:%3.1f\r", minT, meanT, maxT);
        puts("\r\n");

        wait_ms(500);
    }
}

