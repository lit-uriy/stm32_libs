#define MULTIPLE_PROBES
#define DATA_PIN        A0


#ifdef MULTIPLE_PROBES

#include "mbed.h"
#include "DS1820.h"

#define MAX_PROBES      16
 
DS1820* probe[MAX_PROBES];
 
int main() {  
    // Initialize the probe array to DS1820 objects
    int num_devices = 0;
    while(DS1820::unassignedProbe(DATA_PIN)) {
        probe[num_devices] = new DS1820(DATA_PIN);
        num_devices++;
        if (num_devices == MAX_PROBES)
            break;
    }
    
    printf("Found %d device(s)\r\n\n", num_devices);
    while(1) {
        probe[0]->convertTemperature(true, DS1820::all_devices);         //Start temperature conversion, wait until ready
        for (int i = 0; i<num_devices; i++)
            printf("Device %d returns %3.1foC\r\n", i, probe[i]->temperature());
        printf("\r\n");
        wait(1);
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