#include "eint.h"
#include <avr/io.h>
#include <avr/interrupt.h>

int setEint(InterruptPin eint,Mode mode){
    if((eint>=0)&&(eint<=2)){
        //configure pin
        EICRA |= mode<<(2*eint);

    }
    else return 1;
    EIFR = 1<<eint;     //clear interrupt register
    EIMSK |=1<<eint;    //Set interrupt mask
    return 0;
}

int clearEint(InterruptPin eint){
    if((eint>=0)&&(eint<=2)){
        //configure pin
        EIMSK &=~(0x1<<eint);   //disable interrupt flag
        EICRA |= ~(0x3<<(2*eint)); //clear all bytes in EICRx

    }

    else return 1;

    return 0;
}
