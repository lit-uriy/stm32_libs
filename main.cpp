#define MULTIPLE_PROBES
#define DATA_PIN        A0


#ifdef MULTIPLE_PROBES

#include "mbed.h"
#include "DS1820.h"

#define MAX_PROBES      16
 
DS1820 *probe[MAX_PROBES];

DigitalIn mybutton(USER_BUTTON);
 
int main() {  

	printf("\r\n----------------------------\r\n");


    if (mybutton == 0) { // Button is pressed
        printf("Button is pressed\r\n");

    }else{

        printf("Finding devices...\r\n");

        // Initialize the probe array to DS1820 objects
        int num_devices = 0;
        while(DS1820::unassignedProbe(DATA_PIN)) {
            printf("Found %d device\r\n\n", num_devices+1);
            probe[num_devices] = new DS1820(DATA_PIN);
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
