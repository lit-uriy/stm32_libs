

#include "ylist.h"
#include "new_one_wire.h"
#include "one_wire_mbed_pin.h"
#include "mbed.h"



#define DATA_PIN        A0
#define MAX_PROBES      16


int main()
{
    printf("Test with mBed %d.%d.%d\r\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

    // ***** Реализация физического (Physical) уровня и звена данных (Link) 1-Wire

    // -- программная, через ножку контроллера
#if (1)
    DigitalInOut *pin = new DigitalInOut(DATA_PIN); // Mbed-ножка контроллера
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

    bool ok = wire.findDevices(&roms);
    if (!ok){
        printf("Error ocured during the search 1-Wire devices; ErrorCode: %d\r\n", wire.errorCode());
    }

    if (!roms.isEmpty()){
        printf("Found %d device(s):\r\n", roms.size());
        for(int i = 0; i < roms.size(); i++){
            printf("Device #%d: %s\r\n", i, roms.at(i)->romString());
        }
    }else{
        printf("No devices found!\r\n");
    }

}

