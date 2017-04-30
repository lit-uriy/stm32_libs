#include "bin2hex.h"

void bin2hex(unsigned char bin[], unsigned char hex[], int size)
{
    for(size=0; size<8; size++){
        char cl = bin[size] & 0x0F;
        char cm = bin[size] >> 4;
        char resl = 0;
        char resm = 0;

        if (cm <= 9){ // числами
            resm = 0x30 + cm;
        }else{ // буквами
            resm = 55 + cm;
        }

        hex[2*size] = resm;

        if (cl <= 9){ // числами
            resl = 0x30 + cl;
        }else{ // буквами
            resl = 0x37 + cl;
        }

        hex[2*size+1] = resl;
    }
    hex[8*2] = 0x00;
}
