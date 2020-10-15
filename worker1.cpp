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
            nextPrintNext();
        else if (buttonPrintAllTemp)
            nextPrintAll();
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
    OneWireDevice dev = wire::findDevice();
    Yds1820 *thermo = static_cast<Yds1820 *>(&dev);

    printf("\r\nDevice %s:\r\n"
           "\tis parasite powered: %s\r\n"
           "\tresolution = %d\r\n",
           dev.romCode.toString(),
           dev.isParasiticPower() ? "true" : "false",
           thermo? thermo.resolution(): -1);
}






//===========================================================================================
//  Продолжаем поиск следующего 1-Wire устройства
//===========================================================================================
void Worker1::nextFind()
{
    // WARNING: Здесь не реальный код, а план кода!
    if (wire.hasNextDevice()){
        OneWireDevice dev = wire.nextDevice();
        list << dev;
        Yds1820 *thermo = static_cast<Yds1820 *>(&dev);

        printf("\r\nDevice %s:\r\n"
               "\tis parasite powered: %s\r\n"
               "\tresolution = %d\r\n",
               dev.romCode.toString(),
               dev.isParasiticPower() ? "true" : "false",
               thermo? thermo.resolution(): -1);
    }
}






//===========================================================================================
//  Поиск всех 1-Wire устройств
//===========================================================================================
void Worker1::findAll()
{
    // WARNING: Здесь не реальный код, а план кода!
    ;
    while (wire.hasNextDevice()) {
        OneWireDevice dev = wire.nextDevice();
        list << dev;
        Yds1820 *thermo = static_cast<Yds1820 *>(&dev);

        printf("\r\nDevice %s:\r\n"
               "\tis parasite powered: %s\r\n"
               "\tresolution = %d\r\n",
               dev.romCode.toString(),
               dev.isParasiticPower() ? "true" : "false",
               thermo? thermo.resolution(): -1);
    }
}






//===========================================================================================
//  Печатаем температуру следующего термометра
//===========================================================================================
void Worker1::nextPrintNext()
{

}






//===========================================================================================
//  Печатаем температуры всех термометров
//===========================================================================================
void Worker1::nextPrintAll()
{

}

