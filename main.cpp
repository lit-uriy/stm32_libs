#define MULTIPLE_PROBES
#define DATA_PIN        A0


#ifdef MULTIPLE_PROBES

#include "mbed.h"
#include "DS1820.h"
#include "Containers/ylist.h"

#include "1Wire/one_wire.h"

#define MAX_PROBES      16
 
DS1820 *probe[MAX_PROBES];

DigitalIn mybutton(USER_BUTTON);

DigitalOut syncroPin(A1);
bool test = false;
 
int main() {  

    printf("\r\n----------------------------\r\n");

char rom[2*8+1]; // в два раза больше символов + замыкающий нуль

    if (mybutton == 0) { // Button is pressed
        printf("Button is pressed\r\n");

        OneWire wire(DATA_PIN);
        OneWire::LineStatus status = wire.reset();
        if (status == OneWire::StatusPresence){
            wire.readROM();
            printf("Device validity = %d, after Read ROM\r\n", wire.isValid());

            wire.romCode(rom);
            bool ok = wire.isValid();

            printf("ROM status: %d, code = %s\r\n", int(ok), rom);

            ok = false;
        }else{
            printf("Device status after Reset: %d, validity = %d\r\n", int(status), wire.isValid());
        }

        OneWire *dev1 = new OneWire(DATA_PIN);
        OneWire *dev2 = new OneWire(DATA_PIN);
        OneWire *dev3 = new OneWire(DATA_PIN);

        YList<OneWire*> list;
        printf("list create; size = %d\r\n", list.size());
        list.append(dev1);
        list.append(dev2);
        list.append(dev3);
        printf("list populate; size = %d\r\n", list.size());
        int index = list.indexOf(dev2);
        printf("list index of dev2 item: %d\r\n", index);
        index += 1;
        bool eq = list[index] == dev3;
        printf("list index %d, equal dev3?: %d\r\n", index, eq);

    }else{

        printf("Finding devices...\r\n");
#ifdef DS1820_TEST
        DS1820 dev(DATA_PIN);
        dev.romCode(rom);
        printf("ROM code = %s\r\n", rom);
#else
        // Initialize the probe array to DS1820 objects
        int num_devices = 0;
        while(DS1820::unassignedProbe(DATA_PIN)) {
            DS1820 *dev = new DS1820(DATA_PIN);
            probe[num_devices] = dev;
			dev->romCode(rom);
            printf("Found %d device, ROM=%s\r\n\n", num_devices+1, rom);
            num_devices++;
            if (num_devices == MAX_PROBES)
                break;
        }
        if (num_devices){
            printf("Found %d device(s)\r\n\n", num_devices);
            while(1) {
                probe[0]->convertTemperature(true, DS1820::all_devices);         //Start temperature conversion, wait until ready
                for (int i = 0; i<num_devices; i++)
                    printf("Device %d returns %3.1f %sC\r\n", i, probe[i]->temperature(), (char*)(248));
                printf("\r\n");
                wait(1);
            }
        }else{
            error("No devices!\r\n");

            while(1){wait(1);};
        }
#endif
    }
}

#else
#include "mbed.h"
#include "DS1820.h"
 
DS1820 probe(DATA_PIN);
 
int main() {
    while(1) {
        probe.convertTemperature(true, DS1820::all_devices);         //Start temperature conversion, wait until ready
        printf("It is %3.1foC\r\n", probe.temperature());
        wait(1);
    }
}

#endif
