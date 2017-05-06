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
void test3_1();
void test4();
void test5();
 
int main() {  

    printf("\r\n----------------------------\r\n");

char romString[2*8+1]; // в два раза больше символов + замыкающий нуль



    if (mybutton == 0) { // Button is pressed
        printf("Button is pressed\r\n");



//        test1();
//        test2();
        test3();
//        test3_1();
//        test4();
//        test5();

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
    printf("test1\r\n");
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
    printf("test2\r\n");
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
    printf("test3\r\n");
    OneWire wire(DATA_PIN);
    OneWireRomCode romCode;
    romCode = wire.findSingleDevice();
    if (romCode.isNull()){
        if (wire.status() == OneWire::StatusAbsent) {
            printf("Device not found\r\n");
        } else {
            printf("Finding device ERROR=%d, status=%d\r\n", wire.errorCode(), wire.status());
        }
        return;
    }else{
        printf("Found device %s\r\n", romCode.romString());
    }
    // инициализируем термометр полученным ROM-кодом
    Yds1820 thermo(romCode, &wire); // 1-ый способ инициализации
    bool parasite = thermo.isParasiticPower();
    printf("\r\nDevice %s:\r\n"
           "\tis parasite powered: %s\r\n"
           "\tresolution = %d\r\n",
           romCode.romString(),
           parasite ? "true" : "false",
           thermo.resolution());

    while (1) {
        // запускаем преобразование температуры у этого термометра
        int ret = thermo.convertTemperature();
        if (ret < 0){
            if (wire.status() == OneWire::StatusAbsent) {
                printf("\r\nDevice unconected\r\n");
                return; // тут нельзя продолжать, без инициализации.
            } else {
                printf("\r\nDevice %s:\r\n"
                       "\tconvert temperature ERROR, ret=%d\r\n"
                       "\twire ERROR, code=%d, status=%d\r\n",
                       romCode.romString(),
                       ret,
                       wire.errorCode(),
                       wire.status());

            }
            continue;
        }
        float temp = thermo.temperature();
        printf("Device %s, T=%3.1f, resolution=%d\r", romCode.romString(), temp, thermo.resolution());
//        wait(1);
    }
}

// как может выглядеть чтение температуры, с одного единственного датчика
// который никогда НЕ отваливается.
// предполагается, что эта функция вызывается только раз
void test3_1()
{
    printf("test3_1\r\n");
    OneWire wire(DATA_PIN);
    OneWireRomCode romCode;
    romCode = wire.findSingleDevice();
    if (romCode.isNull() && (wire.errorCode() == OneWire::ErrorNon)){
        printf("Device not found\r\n");
        return;
    }else if (wire.errorCode() != OneWire::ErrorNon){
        printf("Finding device ERROR: %d\r\n", wire.errorCode());
    }else{
        printf("Found device %s\r\n", romCode.romString());
    }
    // инициализируем термометр полученным ROM-кодом
    Yds1820 thermo(romCode, &wire); // 1-ый способ инициализации
    while (1) {
//        syncroPin.write(1);
        // запускаем преобразование температуры у этого термометра
        int ret = thermo.convertTemperature();
//        syncroPin.write(0);
        if (ret < 0){
            printf("Device %s, convert temperature ERROR, ret=%d\r\n", romCode.romString(), ret);
            printf("\t, wire ERROR, code=%d, status=%d\r\n", wire.errorCode(), wire.status());
            continue;
        }
        printf("Device %s, converted temperature, time deleay =%d\r\n", romCode.romString(), ret);
        wait(1);
    }
}


// как может выглядеть непрервное чтение температуры, с произвольного числа датчиков
// которые никогда НЕ отваливаются.
// предполагается, что эта функция вызывается только раз
void test4()
{
    printf("test4\r\n");
    OneWire wire(DATA_PIN);
    YList<OneWireRomCode> romCodes;
    romCodes = wire.findMultipleDevices();
    if (romCodes.isEmpty()){
        if (wire.status() == OneWire::StatusAbsent) {
            printf("Devices not found\r\n");
        } else {
            printf("Finding devices ERROR=%d, status=%d\r\n", wire.errorCode(), wire.status());
        }
        return;
    }
    // инициализируем термометры полученными ROM-кодами
    for (int i = 0; i < romCodes.count(); ++i) {
        OneWireRomCode romCode = romCodes.at(i);
        // 2-ой способ инициализации термометра и связывания его с проволокой
        /* TODO: второй способ ещё предстоит реализовать
        Yds1820 *thermo = new Yds1820(rom);
        wire.addDevice(thermo);
        bool parasite = thermo.isParasiticPower();
        */

        // 1-ый способ инициализации
        Yds1820 thermo(romCode, &wire);
        bool parasite = thermo.isParasiticPower();
        printf("\r\nDevice %s:\r\n"
               "\tis parasite powered: %s\r\n"
               "\tresolution = %d\r\n",
               romCode.romString(),
               parasite ? "true" : "false",
               thermo.resolution());
    }
    // запускаем преобразование температуры сразу у всех термометров
    // сидящих на одной проволоке
    int ret = Yds1820::convertTemperature(&wire);
    if (ret < 0){
        if (wire.status() == OneWire::StatusAbsent) {
            printf("\r\nDevice unconected\r\n");
        } else {
            printf("\r\nWire convert temperature ERROR, "
			       "ret=%d\r\n"
                   "\twire ERROR, code=%d, status=%d\r\n",
                   ret,
                   wire.errorCode(),
                   wire.status());

        }
        return;
    }
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
    printf("test5\r\n");
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

