#include "crc.h"

unsigned char crc8(unsigned char data, unsigned char crc8val)
{
    unsigned char cnt;
    cnt=8;
    do{
        if ((data^crc8val)&0x01){
            crc8val^=0x18; //0x18-Полином:0b00011000=>X^8+X^5+X^4+1
            crc8val>>=1;
            crc8val|=0x80;}
        else {
            crc8val>>=1;
        }
        data>>=1;
    }while (--cnt);
    return crc8val;
}
