#include "timer.h"


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

void timer0start(){
    uint8_t prescalevalue = 0x3;
    TCCR0B|=prescalevalue<<CS00;
}

void timer0clear(){
    TCNT0 = 0;
}
void timer1start(){
    uint8_t prescalevalue=4; //prescale value fclk/256
    TCCR1B |= prescalevalue<<CS10; //Timer Enable
}

void timer0intEnable(){
    TIMSK0 |= (1<<TOIE0);
}

void timer0intDisable(){
    TIMSK0 &= ~(1<<OCIE0B|1<<OCIE0A|1<<TOIE0);  //clear all interrupts
}

void timer0stop(){
    TCCR0B &= ~(0x7);


}

void timer1stop(){
    TCCR1B &= ~(0x7);
}


void timer0counterInit(){

    //Clear timer on compare match mode, OCRA
    TCCR0A=(1<<WGM01)|(0<<WGM00)|(0<<COM0A0)|(0<<COM0B0); //Timer stopped
    OCR0A=prescaleTimer0;
    TIMSK0&=~(0x3);                  //clear TIMSK register, in bits from timer 0
    TIMSK0|=1<<TOIE0|1<<OCIE0A;       //program TIMSK register, interruption on top and overflow

}

void timer1counterInit(){
    //Resolution 1 tic = 16 us => 1 s before ovf
    //interruption on ICP
    DDRD&=~(6<<PD4); //for ICP1


    uint8_t wgmvalue = 0; //Normal mode, count till 0xFFFF
    uint8_t filter = 1;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        TCCR1B=((filter&1)<<ICNC1)|(1<<ICES1)|(((wgmvalue>>3)&0x1)<<WGM13)|(((wgmvalue>>2)&0x1)<<WGM12)|(0<<CS10);   //Timer Stopped
        TCCR1A=(DISCONNECTED<<COM1B0)|(DISCONNECTED<<COM1A0)|(((wgmvalue>>1)&0x1)<<WGM11)|((wgmvalue&0x1)<<WGM10);
        TIMSK1&=~(0x27);   //clear TIMSK bits for Timer 1, lack of interruption
        TIMSK1|=1<<TOIE1|1<<ICIE1;    //overflow interrupt/int on pin
    }




}

void timer0pwmInit(){
    //Warning, atm644 has 2 OC0, but atm128 has only one...
    //you also should have your set pins...
    uint8_t wgmvalue=3;   //Fast PWM set
    DDRB |= 1<<PB4|1<<PB3;
    TCCR0A = (wgmvalue&0x3)<<WGM00|NONINVERTING<<COM0A0|NONINVERTING<<COM0B0;
    TCCR0B = ((wgmvalue>>2)&1)<<WGM02;
    uint8_t pwmvalue = 10;
    uint16_t regvalue = ((uint16_t)pwmvalue*255)/100;
    OCR0A = (uint8_t)regvalue;
    OCR0B = (uint8_t)regvalue;
    TIMSK0 &= ~(1<<OCIE0B|1<<OCIE0A|1<<TOIE0);  //clear all interrupts as defeult
    TCCR0B|=0x0<<CS00;   //timer stopped

}



void timer0pwmModify(uint8_t pwmvalue,pwmMode mode, uint8_t ledcolor){
    //Warning -> read warnings for timer0pwmInit
    TCCR0B &= ~0x7;  //timer stop
    if(mode == PERCENT){
        uint16_t regvalue = ((uint16_t)pwmvalue*255)/100;
        OCR0B = (ledcolor&BLUE?(uint8_t)regvalue:0);
        OCR0A = (ledcolor&RED?(uint8_t)regvalue:0);
    }
    else{
        OCR0B = (ledcolor&BLUE?pwmvalue:0);
        OCR0A = (ledcolor&RED?pwmvalue:0);
    }
    TCCR0B |= 0x3 <<CS00; //timer start
}


void timer2pwmInit(){
    //Warning, atm644 has 2 OC0, but atm128 has only one...
    //you also should have your set pins...
    uint8_t wgmvalue=3;   //Fast PWM set
    //Only OC2A
    DDRD |= 1<<PD7;
    TCCR2A = (wgmvalue&0x3)<<WGM20|NONINVERTING<<COM2A0|NONINVERTING<<COM2B0;
    TCCR2B = ((wgmvalue>>2)&1)<<WGM22;
    uint8_t pwmvalue = 50;
    uint16_t regvalue = ((uint16_t)pwmvalue*255)/100;
    OCR2A = (uint8_t)regvalue;
    TIMSK2 &= ~(1<<OCIE2A|1<<OCIE2B|1<<TOIE2);  //clear all interrupts
    //TIMSK2 |= 1<<TOIE2;  //set overflow interrupt
    TCCR2B|=0x5<<CS20;   //timer start

}



void timer2pwmModify(uint8_t pwmvalue,pwmMode mode){
    //Warning -> read warnings for timer0pwmInit
    TCCR2B &= ~0x7;  //timer stop
    if(mode == PERCENT){
        uint16_t regvalue = ((uint16_t)pwmvalue*255)/100;
        OCR2A = regvalue;
    }
    else{
        OCR2A= pwmvalue;
    }
    TCCR2B |= 0x5 <<CS20; //timer start
}

/*
void timer1pwmInit(){
    cli();
    //Set OC1A as output
    DDRB |= 1<<PB5;
    //WGM 3:0 0111 FASTPWM 10 bits
    //10 bits -> 1024
    unsigned char wgmvalue = 6;

    TCCR1B=(((wgmvalue>>3)&0x1)<<WGM13)|(((wgmvalue>>2)&0x1)<<WGM12)|(0<<CS10);   //Timer Stopped
    TCCR1A=(DISCONNECTED<<COM1C0)|(DISCONNECTED<<COM1B0)|(NONINVERTING<<COM1A0)|(((wgmvalue>>1)&0x1)<<WGM11)|((wgmvalue&0x1)<<WGM10);
    int pwmvalue = 50;
    int regvalue = (pwmvalue*511)/100; //512 -1 for wgmvalue == 6
    OCR1A = regvalue&0xFFFF;
    OCR1B=0x0;
    OCR1C=0x0;
    TIMSK&=~(0xF<<2);   //clear TIMSK bits for Timer 1, lack of interruption

    TCCR1B|=0x3<<CS10;  //timer start
    sei();
}

void timer1pwmModify(unsigned int pwmvalue){
    int regvalue = (pwmvalue*511)/100; //512 -1 for wgmvalue == 6
    cli();
    TCCR1B&=~0x7;   //Timer Stop
    OCR1AH = (regvalue>>8)&0xFF;
    OCR1AL = (regvalue)&0xFF;
    sei();
    TCCR1B|=0x3<<CS10;  //Timer Start

}
*/
