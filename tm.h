/**************************************************************************
*================== НПО "Сибирский арсенал" ==============================*
***************************************************************************
* 	
* Заголовок:			функции TM
* Имя файла:			tm.h
*  
* Версия программы:		1.0
* Компилятор:			WinAVR (avr-gcc rev3.3.1)
* Микросхема:			ATmega8535
* Частота:				6.000МГц
* Дата начала:			05.10.2006	
* Последнее обновление:	06.10.2006
* Автор:				Литкевич Юрий Иванович
* Support E-mail:		lit-uriy@yandex.ru
*
* Описание:				
* 							
* 							
* 							
* Log:
* 
*
*
*********************************************************/
#ifndef TM_H
#define TM_H
#include "global.h"


/************** Type ************************************/



/************* Const/Macro ******************************/
#define TM_LOW		{TM_PORT&=~(1<<TM_PIN);TM_DIR|=(1<<TM_PIN);} //настроить на выход и выставить нуль
#define TM_RELEASE	{TM_PORT&=~(1<<TM_PIN);TM_DIR&=~(1<<TM_PIN);} //настроить на вход без подтяжки
#define TM			(TM_PORT_IN&(1<<TM_PIN))


#define T_SLOT	18			//длительность слота
#define T_10U	(T_SLOT/6)	//10мкс
#define T_RSTL	(8*T_SLOT)	//длительность сброса (reset)
#define T_RELS	(T_SLOT*2/5)//длительность востановления (release)
#define T_PRES	(4*T_SLOT)	//длительность отклика (presence)



#define delay_TM(del)	TMR2Delay(del)//задержка

#define TM_PRESNC		0x00	//TM присутствует
#define TM_SHRT_CRT		0x01	//Линия TM закорочена
#define TM_ABSNT		0x02	//TM отсутствует

#define TM_CMD_READROM		0x33	//Чтение ПЗУ (ID)
#define TM_CMD_MATCHROM		0x55	//Обращение по адресу
#define TM_CMD_SEARCHROM	0xF0	//поиск ПЗУ (ID)
#define TM_CMD_SKIPROM		0xCC	//пропустить ПЗУ (ID)

/************** Variables *******************************/
byte TMbuf[8];	
byte TMcrc;


/***************** FuncProto ****************************/

byte TMByte(byte* x);
byte TMReset(void);

#endif /* TM_H */
