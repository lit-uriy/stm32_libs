
#include "../mbed/mbed.h"
#include "../DS1820/DS1820.h"



#define DATA_PIN        A0
#define MAX_PROBES      16



DigitalIn mybutton(USER_BUTTON);
//bool mybutton = true;

DigitalOut syncroPin(LED2);
//bool test = false;

DS1820* makeDevice(PinName name, int num_devices);

void printTemperature(DS1820 *dev, int num_devices);
void printRam(DS1820 *dev, int num_devices);

int main() {

    printf("\r\n----------------------------\r\n");

    if (mybutton) {
        printf("Button is not pressed, Finding multiple devices...\r\n");
        DS1820 *probe[MAX_PROBES];

        // Initialize the probe array to DS1820 objects
        int num_devices = 0;
        while(DS1820::unassignedProbe(DATA_PIN)) {
            num_devices++;
            DS1820 *dev = makeDevice(DATA_PIN, num_devices);
            probe[num_devices-1] = dev;
            if (num_devices == MAX_PROBES)
                break;
        }
        if (num_devices){
            printf("Found %d device(s)\r\n\n", num_devices);
            while(1) {
                probe[0]->convertTemperature(true, DS1820::all_devices);         //Start temperature conversion, wait until ready
                for (int i = 0; i<num_devices; i++){
                    printRam(probe[i], i+1);
                    printTemperature(probe[i], i+1);
                    printRam(probe[i], i+1);
                }
                printf("\r\n");
                wait(1);
            }
        }else{
            error("No devices!\r\n");

            while(1){wait(1);};
        }

    }else {// Button is pressed
        printf("Button is pressed, Finding single devices...\r\n");

        bool ok = DS1820::unassignedProbe(DATA_PIN);
        printf("UnassignedProbe, return %s\r\n", ok? "true": "false");

        DS1820 *probe = makeDevice(DATA_PIN, 1);

        printf("Found %d device(s)\r\n\n", 1);

        while(1) {
            probe->convertTemperature(true, DS1820::all_devices);         //Start temperature conversion, wait until ready
            printRam(probe, 1);
            printTemperature(probe, 1);
            printRam(probe, 1);
            printf("\r\n");
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
    printRam(dev, num_devices);
    printf("\r\n");

    return dev;
}

void printTemperature(DS1820 *dev, int num_devices)
{
    printf("Device %d returns %3.1f %sC\r\n", num_devices, dev->temperature(), (char*)(248));
}


void printRam(DS1820 *dev, int num_devices)
{
    char ramString[2*9+1]; // в два раза больше символов + замыкающий нуль
    dev->ramToHex(ramString);
    printf("Device %d RAM: %s\r\n", num_devices, ramString);
}
