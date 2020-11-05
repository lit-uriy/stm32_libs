
#include "mbed.h"
#include "DS1820.h"



#define DATA_PIN        A0
#define MAX_PROBES      16



DigitalIn mybutton(USER_BUTTON);
//bool mybutton = true;

DigitalOut syncroPin(NC);
DigitalOut led(LED2);
//bool test = false;

DS1820* makeDevice(PinName name, int num_devices);

void convertTemperature(DS1820 *dev, int num_device);

void printTemperature(float temp, int num_devices);
void printRam(DS1820 *dev, int num_devices);

void onSerialInput();

Serial port(USBTX, USBRX);
volatile bool newCommand = false;
const int Size = 80;
int receivedBytes = 0;
char commandBuffer[Size];

enum Command {
    CommandTest1 = 0,
    CommandTest2,
    CommandReset,
    CommandUnknown = 0xFF
};

volatile int command = CommandUnknown;

int main() {

//    port.set_flow_control(Serial::Disabled);
    port.attach(&onSerialInput, Serial::RxIrq);

    port.puts("\r\n------------ Ready ----------------\r\n");
      DS1820 *probes[MAX_PROBES];
    int num_devices = 0;

    while(1){
        led = !led;

        if (!newCommand){
            wait(1);
            continue;
        }

        if(strncmp(commandBuffer , "test1()", sizeof("test1()")) == 0){
            command = CommandTest1;
        }else if (strncmp(commandBuffer , "test2()", sizeof("test2()")) == 0){
            command = CommandTest2;
        }else if (strncmp(commandBuffer , "reset()", sizeof("reset()")) == 0){
            command = CommandReset;
        }else{
            command = CommandUnknown;
        }

        if (command == CommandTest1){
            port.puts("Command test1(): Finding multiple devices...\r\n");
            newCommand = false;

						float minT = 0;
						float maxT = 0;
						float meanT = 0;

            // Initialize the probe array to DS1820 objects
            while(DS1820::unassignedProbe(DATA_PIN)) {
                num_devices++;
                DS1820 *dev = makeDevice(DATA_PIN, num_devices);
                probes[num_devices-1] = dev;
                if (num_devices == MAX_PROBES)
                    break;
            }
            if (num_devices){
                port.printf("Found %d device(s)\r\n\n", num_devices);

                while(1) {
                    convertTemperature(probes[0], 0);
									  maxT = -100;
									  minT = 100;
									  meanT = 0;
                    float temp = 0;
                    for (int i = 0; i<num_devices; i++){
                        float t = probes[i]->temperature();
                        temp += t;
											  if (t < minT)
													  minT = t;
											  if (t > maxT)
													  maxT = t;
                        printTemperature(t, i+1);
                    }
										meanT = temp/num_devices;
                    port.printf("Temperature: Min:%3.1f | Mean:%3.1f | Max:%3.1f\r", minT, meanT, maxT);
                    port.puts("\r\n");

                    if (newCommand){
                        break;
                    }
                }
            }else{
//                error("No devices!\r\n");
                port.printf("No devices!\r\n");
            }

        }else if (command == CommandTest2){
            port.puts("Command test2(): Finding single devices...\r\n");
            newCommand = false;

            DS1820 *dev = makeDevice(DATA_PIN, 1);

            port.printf("Found %d device(s)\r\n\n", 1);

            while(1) {
                convertTemperature(dev, 1);
                printTemperature(dev->temperature(), 1);
                port.puts("\r\n");

                if (newCommand)
                    break;
            }
        }else if (command == CommandReset){
            int size = DS1820::getProbes().length();
            port.printf("Command reset(): deleting %d devices from list\r\n", size);
            DS1820::clearProbes();
            num_devices = 0;
            size = DS1820::getProbes().length();
            port.printf("\t left %d devices in list\r\n", size);
            newCommand = false;
        }else{
            port.printf("Unknown command: %s\r\n", commandBuffer);
            newCommand = false;
        }

        port.puts("\r\n------------ Ready ----------------\r\n");
    }
}



void onSerialInput()
{
    do{
        char c = port.getc();
        if ((receivedBytes >= Size) || newCommand){
            continue;
        }else if ((c == '\n') || (c == '\r')){
            newCommand = true;
            commandBuffer[receivedBytes++] = 0;
            receivedBytes = 0;
            break;
        }
        commandBuffer[receivedBytes++] = c;
    }while(port.readable());
}


DS1820* makeDevice(PinName name, int num_devices)
{
    char romString[2*8+1]; // в два раза больше символов + замыкающий нуль

    DS1820 *dev = new DS1820(name);
    dev->romCode(romString);
    port.printf("Found %d device, ROM=%s\r\n", num_devices, romString);
    port.printf("\tparasite powered: %s\r\n", dev->isParasitePowered()? "Yes": "No");
    char ramString[2*9+1]; // в два раза больше символов + замыкающий нуль
    dev->ramToHex(ramString);
    port.printf("\tRAM: %s\r\n", ramString);
    port.printf("\tresolution: %d bits\r\n", dev->resolution());
    port.printf("\r\n");

    return dev;
}

void convertTemperature(DS1820 *dev, int num_device)
{
    dev->convertTemperature(true, DS1820::all_devices);         //Start temperature conversion, wait until ready
}

void printTemperature(float temp, int num_devices)
{
    port.printf("Device %d returns %3.1f %sC\r\n", num_devices, temp, (char*)(248));
}


void printRam(DS1820 *dev, int num_devices)
{
    char ramString[2*9+1]; // в два раза больше символов + замыкающий нуль
    dev->ramToHex(ramString);
    port.printf("Device %d RAM: %s\r\n", num_devices, ramString);
}
