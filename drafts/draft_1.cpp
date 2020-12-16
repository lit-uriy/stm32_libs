

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

Yds1820* makeDevice2(YTextStream *out, OneWireRomCode romCode, OneWire *awire, int num_devices);

OneWireRomCode stringToRomCode(const char *string);

void convertTemperature1(OneWire *awire);
void convertTemperature2(Yds1820 *dev);

void printTemperature(float temp, int num_devices);

void onSerialInput(YIODevice *device);


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
    YTextCodec *codec = YTextCodec::codecForName("utf-8");

    YSerialPort port(GpioPinName(USBTX), GpioPinName(USBRX));
    port.setBoderate(9600);
    port.setBits(8);
    port.setParity(YSerialPort::ParityNone);
    port.setStops(1.0);
    port.setCallback(YSerialPort::ReadyRead, onSerialInput);

    YTextStream out(&port); // YTextStream(YIODevice *device)
    out.setCodec(codec);

    if (!port.open(YIODevice::ReadOnly | YIODevice::Text)){
        return;
    }

    const uint16 lcdPortMask = 0x000F;
    YIOPort lcdPort(GpioPortName(GPIOA), lcdPortMask);
    Lcd lcd(lcdPort);

    YTextStream lcdout(&lcd); // YTextStream(YIODevice *device)
    lcdout.setCodec(codec);

    if (!lcdPort.open(YIODevice::ReadOnly | YIODevice::Text)){
        return;
    }

    port.attach(&onSerialInput, Serial::RxIrq);

    out << "\r\n------------ Ready (Готов) ----------------\r\n";
    lcdout << "Ready (Готов)";

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
            out << "Command test1(): Finding multiple Yds1820 devices...\r\n";

            lcdout << Lcd::clear;
            lcdout << "Command test1()";

            newCommand = false;

            float minT = 0;
            float maxT = 0;
            float meanT = 0;

            YList<OneWireRomCode*> roms;

            bool ok = wire.findDevices(&roms);
            if (!ok){
                out << "Error ocured during the search; ErrorCode:";
                out << wire.errorCode();
                out << "\r\n\n";

                lcdout << Lcd::row(2) << Lcd::col(1) << "Search error " << wire.errorCode();
            }

            if (!roms.isEmpty()){
                for(int i = 0; i < roms.size(); i++){
                    unsigned char familyCode = roms.at(i)->familyCode();
                    if ((familyCode == Yds1820::FamilyDs1820) || (familyCode == Yds1820::FamilyDs1822) || (familyCode == Yds1820::FamilyDs18B20)){
                        OneWireRomCode *romCode = roms.at(i);
                        Yds1820 *dev = makeDevice2(out, *romCode, &wire, i+1);
                        probes2.append(dev);
                    }
                }
            }

            if (probes2.size()){
                out << "Found " << probes2.size() << " device(s)\r\n";

                lcdout << Lcd::row(2) << Lcd::col(1) << "Found " << probes2.size() << " device(s)";

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

                    // ?????????????????
                    YString msg = "Temperature: Min:%3.1f | Mean:%3.1f | Max:%3.1f";
                    msg = msg.arg(minT).arg(minT).arg(minT);
                    out << msg << endl;

                    lcdout << Lcd::row(2) << Lcd::col(1) << msg;

                    if (newCommand){
                        break;
                    }
                }
            }else{
//                error("No devices!\r\n");
                port.printf("No devices!\r\n");
            }

        }else if (command == CommandTest2){
            out << "Command test2(): Finding single Yds1820 devices...\r\n";

            lcdout << Lcd::clear;
            lcdout << "Command test2()";

            newCommand = false;

            // 28FF2BA36B180141
            // 28FF0A1C661803D3
            // 28FFF5EC6718017C
            // - на работе
            // 28FFE88E601802E3
            // 28FFF5EC6718017C
            // 28FF63C16B180101

            Yds1820 *dev = makeDevice2(out, stringToRomCode("28FFE88E601802E3"), &wire, 1);
            probes2.append(dev);

            OneWireRomCode rom;
            wire.reset();
            bool ok = wire.readROM(&rom);
            if (ok){
                out << "Found device with ROM: " << rom.romString() << "\n";

                lcdout << Lcd::row(2) << Lcd::col(1) << "Found ROM: " << rom.romString();
            }else{
                out << "Found device with ROM: " << rom.romString();
                out << ", Error code=" << hex << wire.errorCode();

                lcdout << Lcd::row(2) << Lcd::col(1) << "Found ROM: " << rom.romString();
            }

            while(1) {
                convertTemperature2(dev);
                printTemperature(dev->temperature(), 1);
                port.puts("\r\n");

                if (newCommand)
                    break;
            }
        }else if (command == CommandReset){
            out << "Command reset(): deleting " << probes2.size() << " devices from list\r\n";

            lcdout << Lcd::clear;
            lcdout << "Command reset(): deleting " << probes2.size() << " devices from list\r\n";

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



void onSerialInput(YIODevice *device)
{
    do{
        char c = 0;
        device.read(&c, 1);
        if ((receivedBytes >= Size) || newCommand){
            continue;
        }else if ((c == '\n') || (c == '\r')){
            newCommand = true;
            commandBuffer[receivedBytes++] = 0;
            receivedBytes = 0;
            break;
        }
        commandBuffer[receivedBytes++] = c;
    }while(!device.atEnd());
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


Yds1820 *makeDevice2(YTextStream *out, OneWireRomCode romCode, OneWire *awire, int num_devices)
{
    Yds1820 *dev = new Yds1820(romCode, awire);
    out << "Found " << num_devices << " device, ROM=" << romCode->romString() << "\r\n";
    out << "\tparasite powered: " << (dev->isParasitePowered()? "Yes": "No") << "\r\n";
    out << "\tresolution: " << dev->resolution() << " bits\r\n";
    return dev;
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

