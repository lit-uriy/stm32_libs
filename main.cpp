#include "mbed.h"
#include "Containers/ylist.h"

#include "1Wire/one_wire.h"
#include "1Wire/y_ds1820.h"


DigitalInOut oneWirePin(D7); // ножка для 1-Wire

DigitalIn buttonStartFind(D2); // Начать поиск 1-Wire-устройств
DigitalIn buttonNextFind(D3);  // Поиск следующего 1-Wire-устройства

DigitalIn buttonPrintNextTemp(USER_BUTTON);  // Печать следующего термометра
DigitalIn buttonPrintAllTemp(D4);  // Непрерывное измерение по всем термометрам

DigitalOut ledShortingWire(D8); // сигнал на светодиод о КЗ на 1-Wire

DigitalOut syncroPin(A1);
bool test = false;


char romString[2*8+1]; // в два раза больше символов + замыкающий нуль


void startFind();
void nextFind();
void nextPrintNext();
void nextPrintAll();

 
int main() {  

    printf("\r\n---------- RESET ------------------\r\n");


    while (1) {
        if (buttonStartFind)
            startFind();
        else if (buttonNextFind)
            nextFind();
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
void startFind()
{

}






//===========================================================================================
//  Продолжаем поиск следующего 1-Wire устройства
//===========================================================================================
void nextFind()
{

}






//===========================================================================================
//  Печатаем температуру следующего термометра
//===========================================================================================
void nextPrintNext()
{

}






//===========================================================================================
//  Печатаем температуры всех термометров
//===========================================================================================
void nextPrintAll()
{

}

