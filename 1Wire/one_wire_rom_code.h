#ifndef ONE_WIRE_ROM_CODE_H
#define ONE_WIRE_ROM_CODE_H


class OneWireRomCode
{
public:
    OneWireRomCode()
    {
        for (int i = 0; i < 8; ++i) {
            bytes[i] = 0;
        }
    }

    // TODO: Нужен конструктор, который будет конструировать из строки.

    bool isNull()
    {
        for (int i = 0; i < 8; ++i) {
            if (bytes[i]) return false;
        }
        return true;
    }

    char* romString()
    {
        unsigned char i;

        for(i=0; i<8; i++){
            char cl = bytes[i] & 0x0F;
            char cm = bytes[i] >> 4;
            char resl = 0;
            char resm = 0;

            if (cm <= 9){ // числами
                resm = 0x30 + cm;
            }else{ // буквами
                resm = 55 + cm;
            }

            _romString[2*i] = resm;

            if (cl <= 9){ // числами
                resl = 0x30 + cl;
            }else{ // буквами
                resl = 0x37 + cl;
            }

            _romString[2*i+1] = resl;
        }
        _romString[8*2] = 0x00;
        return _romString;
    }
    unsigned char familyCode()
    {
        return bytes[0];
    }

    bool bit(int bitIndex)
    {
        int byteIndex = bitIndex/8;
        unsigned char byte = bytes[byteIndex];
        int bitInbyte = bitIndex%8;
        return byte & (1 << bitInbyte);
    }

    void setBit(int bitIndex, bool value)
    {
        int byteIndex = bitIndex/8;
        unsigned char byte = bytes[byteIndex];
        int bitInbyte = bitIndex%8;
        unsigned char mask = 1 << bitInbyte;
        bytes[byteIndex] = value? (byte | mask): (byte & ~mask);
    }
    unsigned char bytes[8];
    char _romString[2*8+1]; // в два раза больше символов + замыкающий нуль
};

#endif // ONE_WIRE_ROM_CODE_H
