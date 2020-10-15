#include "tests_1.h"

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
        bool parasite = thermo.isParasiticPower();
        printf("Device %s, T=%3.1f, resolution=%d, isParasite=%s\r",
               romCode.romString(),
               temp,
               thermo.resolution(),
               parasite ? "true" : "false");
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

    // ищем
    OneWire::LineStatus status = wire.findMultipleDevices(&romCodes);
    if (status == OneWire::StatusAbsent) {
        printf("Devices not found, ERROR=%d, status=%d\r\n",
               wire.errorCode(),
               wire.status());
        return;
    } else if (status == OneWire::StatusError){
        printf("Finding devices ERROR=%d, status=%d\r\n",
               wire.errorCode(),
               wire.status());
        return;
    }

    printf("Find devices  %d\r\n", romCodes.count());

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
    // непрерывный опрос термометров
    while (1) {
        // запускаем преобразование температуры сразу у всех термометров
        // сидящих на одной проволоке
        int ret = Yds1820::convertTemperature(&wire);
        if (ret < 0){
            if (wire.status() == OneWire::StatusAbsent) {
                printf("\r\nDevice unconected\r\n");
                return; // тут нельзя продолжать, без инициализации.
            } else {
                printf("\r\nWire convert temperature ERROR, "
                       "ret=%d\r\n"
                       "\twire ERROR, code=%d, status=%d\r\n",
                       ret,
                       wire.errorCode(),
                       wire.status());

            }
            continue;
        }
        // печатаем тепературу каждого термометра
        YList<OneWireDevice *> devices = wire.devices();
        for (int i = 0; i < devices.count(); ++i) {
            Yds1820 *thermo = static_cast<Yds1820 *>(devices.at(i));
            float temp = thermo->temperature();
            printf("Device %d returns %3.1f %sC\r\n", i, temp, (char*)(248));
        }
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
            OneWire::LineStatus status = wire.findMultipleDevices(&romCodes);
            if (status == OneWire::StatusAbsent) {
                continue;
            } else if (status == OneWire::StatusError){
                printf("Finding devices ERROR=%d, status=%d\r\n", wire.errorCode(), wire.status());
                continue;
            }
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

