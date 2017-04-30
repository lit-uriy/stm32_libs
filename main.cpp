#define MULTIPLE_PROBES
#define DATA_PIN        A0


#ifdef MULTIPLE_PROBES

#include "mbed.h"
#include "DS1820.h"
#include "Containers/ylist.h"

#include "1Wire/one_wire.h"
#include "1Wire/y_ds1820.h"

#define MAX_PROBES      16
 
DS1820 *probe[MAX_PROBES];

DigitalIn mybutton(USER_BUTTON);

DigitalOut syncroPin(A1);
bool test = false;

void test1();
void test2();
void test3();
void test4();
void test5();
 
int main() {  

    printf("\r\n----------------------------\r\n");

char romString[2*8+1]; // в два раза больше символов + замыкающий нуль



    if (mybutton == 0) { // Button is pressed
        printf("Button is pressed\r\n");

//        OneWire wire(DATA_PIN);
//        OneWire::LineStatus status = wire.reset();
//        if (status != OneWire::StatusPresence){
//            printf("Device status after Reset: %d\r\n", int(status));
//            exit(-1);
//        }

//        OneWireRomCode romCode = wire.findSingleDevice();

//        Yds1820 probe(romCode);
//        wire.addDevice(&probe);
		
//		bool ok = probe.readROM();
//        if (!ok){
//            printf("Device status after Read ROM: %d\r\n", int(wire.status()));
//            exit(-1);
//        }
//        probe.romString(romString);
//        printf("ROM code = %s\r\n", romString);

//        bool power = probe.readPowerSupply();
//        printf("Device %s power %d\r\n", romString, power);

//        char ramString[2*9+1]; // в два раза больше символов + замыкающий нуль
//        ok = probe.readRam();
//        probe.ramString(ramString);
//        printf("Device %s RAM: %s\r\n", romString, ramString);

//        while(1) {
//            probe.ramString(ramString);
//            printf("Device %s RAM: %s\r\n", romString, ramString);

//            probe.convertTemperature(true, Yds1820::DevicesAll);         //Start temperature conversion, wait until ready

//            float temp = probe.temperature();
//            printf("Device %s returns %3.1f %sC\r\n", romString, temp, (char*)(248));
//            printf("\r\n");
//            wait(1);
//        }

        test1();
        test2();
        test3();
        test4();
        test5();

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
            dev->romCode(romString);
            printf("Found %d device, ROM=%s\r\n\n", num_devices+1, romString);
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


// как может выглядеть ожидание таблетки для программирования в БД
void test1()
{
    bool programmingMode = true;
    OneWire wire(DATA_PIN);
    OneWireRomCode romCode;
	bool buttonPresent = false;
    while (!buttonPresent && programmingMode){
		romCode = wire.findSingleDevice();
		buttonPresent = !romCode.isNull();
	}
    if (romCode.isNull() || !programmingMode)
        return;
    // сохраняем код таблетки в БД
}

// как может выглядеть ожидание таблетки, для открытия двери
// предполагается, что эта функция вызывается регулярно
void test2()
{
    OneWire wire(DATA_PIN);
    OneWireRomCode romCode;
    romCode = wire.findSingleDevice();
    if (romCode.isNull())
        return;
    // проверяем, есть ли такая таблетка в БД, если есть то открываем дверь
}

// как может выглядеть чтение температуры, с одного единственного датчика
// который никогда НЕ отваливаются.
// предполагается, что эта функция вызывается только раз
void test3()
{
    OneWire wire(DATA_PIN);
    OneWireRomCode romCode;
    romCode = wire.findSingleDevice();
    if (romCode.isNull()){
        printf("Device not found\r\n");
        return;
    }
    // инициализируем термометр полученным ROM-кодом
    Yds1820 thermo(romCode, &wire); // 1-ый способ инициализации
    while (1) {
        // запускаем преобразование температуры у этого термометра
        thermo.convertTemperature();
        float temp = thermo.temperature();
        printf("Device %s, T=%d\r\n\n", romCode.romString(), temp);
        wait(1);
    }
}


// как может выглядеть непрервное чтение температуры, с произвольного числа датчиков
// которые никогда НЕ отваливаются.
// предполагается, что эта функция вызывается только раз
void test4()
{
    OneWire wire(DATA_PIN);
    YList<OneWireRomCode> romCodes;
    romCodes = wire.findMultipleDevices();
    if (romCodes.isEmpty())
        return;
    // инициализируем термометры полученными ROM-кодами
    for (int i = 0; i < romCodes.count(); ++i) {
        OneWireRomCode rom = romCodes.at(i);
        // второй способ инициализации термометра и связывания его с проволокой
        Yds1820 *thermo = new Yds1820(rom);
        wire.addDevice(thermo);
    }
    // запускаем преобразование температуры сразу у всех термометров
    // сидящих на одной проволоке
    Yds1820::convertTemperature(&wire);
    // печатаем тепературу каждого термометра
    YList<OneWireDevice *> devices = wire.devices();
    for (int i = 0; i < devices.count(); ++i) {
        Yds1820 *thermo = static_cast<Yds1820 *>(devices.at(i));
        float temp = thermo->temperature();
        printf("Device %d returns %3.1f %sC\r\n", i, temp, (char*)(248));
    }
}


// как может выглядеть непрервное чтение температуры, с произвольного числа датчиков
// с возможностью принудительного поиска новых устройств по кнопке.
// предполагается, что эта функция вызывается только раз
void test5()
{
    OneWire wire(DATA_PIN);
    YList<OneWireRomCode> romCodes;

    while(1){
        if (mybutton){
            romCodes = wire.findMultipleDevices();
            if (romCodes.isEmpty())
                continue;
            // инициализируем термометры полученными ROM-кодами
            for (int i = 0; i < romCodes.count(); ++i) {
                OneWireRomCode rom = romCodes.at(i);
                // второй способ инициализации термометра и связывания его с проволокой
                Yds1820 *thermo = new Yds1820(rom);
                wire.addDevice(thermo);
            }
        }else{
            if (romCodes.isEmpty())
                continue;
            // запускаем преобразование температуры сразу у всех термометров
            // сидящих на одной проволоке
            Yds1820::convertTemperature(&wire);
            // печатаем тепературу каждого термометра
            YList<OneWireDevice *> devices = wire.devices();
            for (int i = 0; i < devices.count(); ++i) {
                Yds1820 *thermo = static_cast<Yds1820 *>(devices.at(i));
                float temp = thermo->temperature();
                printf("Device %d returns %3.1f %sC\r\n", i, temp, (char*)(248));
            }
        }
        // секндная пауза
        wait(1);
    }
}

