#include <avr/io.h>

#include <avr/interrupt.h>
//#include "timer.h"
#include "utils.h"
#include "uart.h"
#include "timer.h"

void ledtype(LEDSTATE ledstate, TOURELLECOLOR  ledclr){

    if(ledstate==CLIGNOTER){
        timer0stop();
        timer0clear();
        timer0intEnable();
        timer0start();
    }
    else{
        timer0stop();
        timer0clear();
        timer0intDisable();
        timer0pwmModify(255,POWTWO,ledclr);
        timer0start();
    }
}

#if USARTFUNCTIONS
#include <util/delay.h>
void usartNumberTest(){
    USART_Init(103);
    sei();
    int tosend=3000;
    while(1){
        _delay_ms(50);

        USART_SendU16(tosend++);

        USART_Transmit('\n');

    }


}
#endif


int abs(signed int a){
	if(a>0) return a;
	else return -a;
}

#if USARTFUNCTIONS
void usartTest(){
    USART_Init(103);
    sei();
    uint8_t tosend=32;
    while(1){
        _delay_ms(50);

        USART_Transmit(tosend++);
        if(tosend==0x7F) {
            tosend=32;
            USART_Transmit('\n');
        }
    }



}
#endif

/*
void clignoter(unsigned char led,unsigned int delay){
    //WARNING, INFITE LOOP!
    DDRA|=0x1<<led;
    static char state=0;
    while(1){
        if(state){
            PORTA&=~(0x1<<led);
            state=0;
                }
        else{
            PORTA|=0x1<<led;
            state=1;
            }
            _delay_ms(delay);
        }
    }

void afichercounter(){
    //WARNING INFINITE LOOP!!
    while(1){
        _delay_ms(50);
        PORTA = TCNT0;

    }
}

void pwmtest(){
    //WARNING INFINITE LOOP!
        //PWM test...
    int i=0;
    sei();
    timer1pwmInit();
    DDRA=0x1F;
    PORTA=(1<<4|1<<2|1);
    _delay_ms(1000);
    PORTA=0x00;
    while(1){
        for(i=0;i<=100;i++){
            _delay_ms(10);

            timer1pwmModify(i);

        }
        for(i=100;i>=0;i--){
            _delay_ms(10);
            timer1pwmModify(i);
        }
    }

}

*/
