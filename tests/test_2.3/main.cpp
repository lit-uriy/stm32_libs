

#include "ylist.h"
#include "one_wire.h"
#include "one_wire_device.h"
#include "y_ds1820.h"
#include "DS1820.h"
#include "mbed.h"



#define DATA_PIN        A0
#define MAX_PROBES      16



DigitalInOut pin(DATA_PIN);
OneWire wire(pin);

DigitalIn mybutton(USER_BUTTON);
//bool mybutton = true;

DigitalOut syncroPin(NC);
DigitalOut led(LED2);
//bool test = false;

DS1820* makeDevice(PinName name, int num_devices);
Yds1820* makeDevice2(OneWireRomCode romCode, OneWire *awire, int num_devices);



OneWireRomCode stringToRomCode(const char *string);

void convertTemperature(DS1820 *dev, int num_device);
void convertTemperature1(OneWire *awire);
void convertTemperature2(Yds1820 *dev);

void printTemperature(float temp, int num_devices);
//void printRam(DS1820 *dev, int num_devices);

void onSerialInput();

Serial port(USBTX, USBRX);
volatile bool newCommand = false;
const int Size = 80;
int receivedBytes = 0;
char commandBuffer[Size];

enum Command {
    CommandTest1 = 0,
    CommandTest2,
    CommandTest3,
    CommandReset,
    CommandUnknown = 0xFF
};

volatile int command = CommandUnknown;

int main() {

//    port.set_flow_control(Serial::Disabled);
    port.attach(&onSerialInput, Serial::RxIrq);

    port.puts("\r\n------------ Ready ----------------\r\n");
    YList<DS1820*> probes;
    YList<Yds1820*> probes2;

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
        }else if (strncmp(commandBuffer , "test3()", sizeof("test3()")) == 0){
            command = CommandTest3;
        }else if (strncmp(commandBuffer , "reset()", sizeof("reset()")) == 0){
            command = CommandReset;
        }else{
            command = CommandUnknown;
        }

        if (command == CommandTest1){
            port.puts("Command test1(): Finding multiple Yds1820 devices...\r\n");
            newCommand = false;

            float minT = 0;
            float maxT = 0;
            float meanT = 0;

            YList<OneWireRomCode*> roms;

            bool ok = wire.findDevices(&roms);
            if (!ok){
                printf("Error ocured during the search; ErrorCode: %d\r\n\n", wire.errorCode());
            }

            if (!roms.isEmpty()){
                for(int i = 0; i < roms.size(); i++){
                    unsigned char familyCode = roms.at(i)->familyCode();
                    if ((familyCode == Yds1820::FamilyDs1820) || (familyCode == Yds1820::FamilyDs1822) || (familyCode == Yds1820::FamilyDs18B20)){
                        Yds1820 *dev = makeDevice2(*(roms.at(i)), &wire, 1);
                        probes2.append(dev);
                    }
                }
            }

            if (probes2.size()){
                port.printf("Found %d device(s)\r\n\n", probes2.size());

                while(1) {
                    convertTemperature1(&wire);
                    maxT = -100;
                    minT = 100;
                    meanT = 0;
                    float temp = 0;
                    for (int i = 0; i < probes2.size(); i++){
                        float t = probes2.at(i)->temperature();
                        temp += t;
                        if (t < minT)
                              minT = t;
                        if (t > maxT)
                              maxT = t;
                        printTemperature(t, i+1);
                    }
                    meanT = temp/probes2.size();
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
            port.puts("Command test2(): Finding single Yds1820 devices...\r\n");
            newCommand = false;

            // 28FF2BA36B180141
            // 28FF0A1C661803D3
            // 28FFF5EC6718017C
            // - на работе
            // 28FFE88E601802E3
            // 28FFF5EC6718017C
            // 28FF63C16B180101
            Yds1820 *dev = makeDevice2(stringToRomCode("28FF2BA36B180141"), &wire, 1);
            probes2.append(dev);

            port.printf("Found %d device(s)\r\n\n", 1);

            while(1) {
                convertTemperature2(dev);
                printTemperature(dev->temperature(), 1);
                port.puts("\r\n");

                if (newCommand)
                    break;
            }
        }else if (command == CommandTest3){
            port.puts("Command test3(): Finding multiple DS1820 devices...\r\n");
            newCommand = false;

                        float minT = 0;
                        float maxT = 0;
                        float meanT = 0;

            // Initialize the probe array to DS1820 objects
            while(DS1820::unassignedProbe(DATA_PIN)) {
                DS1820 *dev = makeDevice(DATA_PIN, probes.size());
                probes.append(dev);
                if (probes.size() == MAX_PROBES)
                    break;
            }
            if (probes.size()){
                port.printf("\n");
                port.printf("----------------------------------------\n");
                port.printf("Found %d device(s)\r\n\n", probes.size());

                while(1) {
                    convertTemperature(probes.at(0), 0);
                    maxT = -100;
                    minT = 100;
                    meanT = 0;
                    float temp = 0;
                    for (int i = 0; i < probes.size(); i++){
                        float t = probes.at(i)->temperature();
                        temp += t;
                        if (t < minT)
                              minT = t;
                        if (t > maxT)
                              maxT = t;
                        printTemperature(t, i+1);
                    }
                    meanT = temp/probes.size();
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
        }else if (command == CommandReset){
            port.printf("Command reset(): deleting %d devices from list\r\n", probes2.size());
            for (int i = 0; i < probes2.size(); ++i) {
                Yds1820 *dev = probes2.at(i);
                delete dev;
            }
            probes2.clear();
            port.printf("\t left %d devices in list\r\n", probes2.size());
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



OneWireRomCode stringToRomCode(const char *string)
{
    OneWireRomCode out;
    const int goodSize = (8*2);
    int i = 0;
    char c;
    // сначала проверим кол-во символов во входной строке
    do {
        c = string[i];
        if (c == 0){ // Меньше чем надо
            printf("stringToRomCode(); Less (%d) chars then required (%d)\n", i, goodSize);
            return out;
        }
        ++i;
    } while (i < goodSize);

    // здесь i = goodSize
    c = string[i];
    if (c != 0){ // Больше чем надо
        printf("stringToRomCode(); More (%d) chars then required (%d)\n", i, goodSize);
        return out;
    }

    // хорошая длина
    i = 0;
    char cm, cl;
    char resm, resl;

    do {
        cm = string[2*i];
        cl = string[2*i+1];

        if (cm <= '9')
            resm = cm - 0x30;
        else
            resm = cm - 0x37;

        if (cl <= '9')
            resl = cl - 0x30;
        else
            resl = cl - 0x37;

        out.bytes[i] = ((unsigned char)resm << 4) | (unsigned char)resl;
        i++;
    } while(i < goodSize);

    return out;
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

Yds1820 *makeDevice2(OneWireRomCode romCode, OneWire *awire, int num_devices)
{
    Yds1820 *dev = new Yds1820(romCode, awire);
    port.printf("Found %d device, ROM=%s\r\n", num_devices, romCode.romString());
    port.printf("\tparasite powered: %s\r\n", dev->isParasitePowered()? "Yes": "No");
//    char ramString[2*9+1]; // в два раза больше символов + замыкающий нуль
//    dev->ramToHex(ramString);
//    port.printf("\tRAM: %s\r\n", ramString);
    port.printf("\tresolution: %d bits\r\n", dev->resolution());
    port.printf("\r\n");

    return dev;
}


void convertTemperature(DS1820 *dev, int num_device)
{
    dev->convertTemperature(true, DS1820::all_devices);         //Start temperature conversion, wait until ready
}


void convertTemperature1(OneWire *awire)
{
    Yds1820::convertTemperature(awire);         //Start temperature conversion, wait until ready
}

void convertTemperature2(Yds1820 *dev)
{
#if (0)
    Yds1820::convertTemperature(&wire);         //Start temperature conversion, wait until ready
#else
    dev->convertTemperature(&wire);         //Start temperature conversion, wait until ready
#endif
}

void printTemperature(float temp, int num_devices)
{
    char celsius = (char)(7); // 176
    port.printf("Device %d returns %3.1f %cC\r\n", num_devices, temp, celsius);
}


//void printRam(DS1820 *dev, int num_devices)
//{
//    char ramString[2*9+1]; // в два раза больше символов + замыкающий нуль
//    dev->ramToHex(ramString);
//    port.printf("Device %d RAM: %s\r\n", num_devices, ramString);
//}
