#ifndef YDS1820_H
#define YDS1820_H

#include "one_wire_device.h"

class Yds1820: public OneWireDevice
{
public:
    enum Devices{
        DeviceThis,     // command applies to only this device
        DevicesAll   // command applies to all devices
    };

    enum {
        invalid_conversion = -1000
    };

    enum FamilyCode {
        FamilyDs1820 = 0x10,
        FamilyDs18B20 = 0x28,
        FamilyDs1822 = 0x22,
    };

    Yds1820(OneWireRomCode aRomCode, OneWire *awire = 0);

    /**
     * Запускает процедуру измерения температуры
     */
    // на ЭТОМ термометре
    int convertTemperature();
    // на всех термометрах,
    // если проволока указана, то в пределах проволоки,
    // а если не указана, то на всех проволоках, на которых есть термометры этого класса
    static int convertTemperature(OneWire *awire = 0);

    float temperature(char scale='c');

    unsigned int resolution();
    bool setResolution(unsigned int res);

    bool readPowerSupply();
    static bool readPowerSupply(OneWire *awire);
    bool isParasitePowered();

    bool readRam();

    bool ramString(char buff[]);

private:
    enum DeviceCommands {
        CommandAlarmSearch = 0xEC, // числится как ROM-команда в PDF-нике, но не в книге по iButton
        // DS18B20 Function Commands
        CommandConvertT = 0x44,
        CommandWriteScratchpad = 0x4E,
        CommandReadScratchpad = 0xBE,
        CommandCopyScratchpad = 0x48,
        CommandRecallEeprom = 0xB8,
        CommandReadPowerSupply = 0xB4,
    };

    union Ram {
        struct {
            short currentTemp;
            char alarmTempHi;
            char alarmTempLow;
            char config;
            char reserved1;
            char reserved2;
            char reserved3;
            char crc;
        };
        char byte[9];
    }_ram;

    bool _parasitePower;
};

#endif // YDS1820_H
