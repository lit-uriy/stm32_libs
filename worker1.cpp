#include "worker1.h"



//===========================================================================================
//  Конфигурация
//===========================================================================================
DigitalInOut oneWirePin(D7); // ножка для 1-Wire

DigitalIn buttonStartFind(D2); // Начать поиск 1-Wire-устройств
DigitalIn buttonNextFind(D3);  // Поиск следующего 1-Wire-устройства
DigitalIn buttonFindAll(D5);  // Поиск всех 1-Wire-устройств подряд

DigitalIn buttonPrintNextTemp(USER_BUTTON);  // Печать следующего термометра
DigitalIn buttonPrintAllTemp(D4);  // Непрерывное измерение по всем термометрам

DigitalOut ledShortingWire(D8); // сигнал на светодиод о КЗ на 1-Wire

DigitalOut syncroPin(A1);








//===========================================================================================
//  Конструктор основного работника
//===========================================================================================
Worker1::Worker1()
    : wire(oneWirePin)
    , currentIndex(-1)
{

}

int Worker1::exec()
{
    while (1) {
        if (buttonStartFind)
            startFind();
        else if (buttonNextFind)
            nextFind();
        else if (buttonFindAll)
            findAll();
        else if (buttonPrintNextTemp)
            printNext();
        else if (buttonPrintAllTemp)
            printAll();
        else
            wait(0.1);
    }
}




//===========================================================================================
//  Начинаем поиск 1-Wire устройств сначала
//===========================================================================================
void Worker1::startFind()
{
    // WARNING: Здесь не реальный код, а план кода!
    list.clear();
    currentIndex = -1;

    OneWireDevice dev = wire::findDevice();
    if (dev.isValid()){
        list << dev;
        currentIndex = 0;

        Yds1820 *thermo = static_cast<Yds1820 *>(&dev);

        printf("\r\nDevice %s:\r\n"
               "\tis parasite powered: %s\r\n"
               "\tresolution = %d\r\n",
               dev.romCode.toString(),
               dev.isParasiticPower() ? "true" : "false",
               thermo? thermo.resolution(): -1);
    }else{
        printf("\r\nNo devices\r\n");
    }
}






//===========================================================================================
//  Продолжаем поиск следующего 1-Wire устройства
//===========================================================================================
void Worker1::nextFind()
{
    // WARNING: Здесь не реальный код, а план кода!
    if (list.isEmpty()){
        startFind();
        return;
    }

    if (wire.hasNextDevice()){
        OneWireDevice dev = wire.nextDevice();
        if (dev.isValid()){
            list << dev;
            Yds1820 *thermo = static_cast<Yds1820 *>(&dev);

            printf("\r\Device found: \r\n"
                   "\tROM: %s\r\n"
                   "\tis parasite powered: %s\r\n"
                   "\tresolution = %d\r\n",
                   dev.romCode.toString(),
                   dev.isParasiticPower() ? "true" : "false",
                   thermo? thermo.resolution(): -1);
        }else{
            printf("\r\nNo more devices\r\n");
        }
    }
}






//===========================================================================================
//  Поиск всех 1-Wire устройств
//===========================================================================================
void Worker1::findAll()
{
    // WARNING: Здесь не реальный код, а план кода!
    startFind();

    while (wire.hasNextDevice()) {
        OneWireDevice dev = wire.nextDevice();
        if (dev.isValid()){
            list << dev;
            currentIndex = 0;
            Yds1820 *thermo = static_cast<Yds1820 *>(&dev);

            printf("\r\Device found: \r\n"
                   "\tROM: %s\r\n"
                   "\tis parasite powered: %s\r\n"
                   "\tresolution = %d\r\n",
                   dev.romCode.toString(),
                   dev.isParasiticPower() ? "true" : "false",
                   thermo? thermo.resolution(): -1);
        }else{
            printf("\r\nNo more devices\r\n");
        }
    }
}






//===========================================================================================
//  Печатаем температуру следующего термометра
//===========================================================================================
void Worker1::printNext()
{
    if (currentIndex >=0){
        OneWireDevice dev = list.at(currentIndex++);
        Yds1820 *thermo = static_cast<Yds1820 *>(&dev);
        float temp = thermo->temperature();

        if (currentIndex >= list.size())
            currentIndex = 0;

        printf("\r\Device [%d]: %3.1f %sC\r\n", currentIndex, temp, (char*)(248));
    }else{
        printf("\r\nNo devices\r\n");
    }
}






//===========================================================================================
//  Печатаем температуры всех термометров
//===========================================================================================
void Worker1::printAll()
{
    currentIndex = 0;
    for (int i = 0; i < list.size(); ++i) {
        printNext();
    }
}

