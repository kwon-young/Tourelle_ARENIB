
#define F_CPU 8000000

#include <stdint.h>

#define RED 1<<0
#define BLUE 1<<1

#define prescaleTimer0 (1e-3*(F_CPU/128))-1
//#define prescaleTimer0 160

typedef unsigned char pwmMode;
#define PERCENT 0
#define POWTWO  1

#define NOPRESCALE 1
#define STDPRESCALE 5



//FLAGS for TIFR timer 0
#define CLEARTOPINTERRUPT  0x1
#define CLEAROVFINTERRUPT  0x2

void timer0init();
void timer1counterInit();

void timer1start();
void timer1stop();

//Flags for COMnx f/ fast PWM
#define DISCONNECTED    0
#define NONINVERTING    2
#define INVERTING       3

/*
void timer1pwmModify(unsigned int pwmvalue);
void timer1pwmInit();
*/

void timer0pwmModify(unsigned char pwmvalue,pwmMode,uint8_t);
void timer0pwmInit();
void timer0stop();
void timer0start();
void timer0intDisable();
void timer0intEnable();
void timer0clear();

void timer2pwmModify(unsigned char pwmvalue,pwmMode mode);
void timer2pwmInit();
