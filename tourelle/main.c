/*
Code tourelle,
by fclad - cladera.f@gmail.com
Viens, viens en Argentine avec nous :D!
18/05/2012
*/

#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <util/atomic.h>
#include <stdint.h>

#include "utils.h"

#include "uart.h"

#include "eint.h"
#include "fifo.h"
#include "i2c.h"


//Configuration
#define DEBUG 0     //Activer code de debug, pour garder les angles obtenus (en mode RAW, sans traitement)
#define ERROR 0     //Activer code de detection d'erreur. Permet de detecter des erreurs et repondre au maître à propos de l'etat de la tourelle
#define RUSHCODE 0  //Activer mode d'emergence, util pour la coupe...
                        /*
                        On envoi sur la pin SCL de l'I2c '1' si on detecte quelque chose proche au robot,
                        sur une plage de +-INTERVAL deg autour 0.
                        ATTENTION: Ce mode n'a pas de com I2c, et doit pas être utilisé si on a com I2c avec d'autres cartes dans le robot
                        */

#define CAPTOR0 3   //le capteur 0 est celui qui voit le plus loin
#define CAPTOR1 1   //le capteur 1 est celui qui voit le plus proche


//Speed values
//--Taille du tampon fifo pour moyenner la vitesse de la tourelle
#define BUFFERSPEED 4

//Angle values
#define NBPOINTS 6          //--Quantité d'angles mesurés en même temps
#define NBREGS 2*NBPOINTS

//Stats values
//--Valeurs statistiques obtenus par essai sur CHAQUE tourelle
    /* CF fichier de calcul ci-joint pour trouver les valeurs a remplir
    */

#define LOWERMARGIN INT16_C(-80)
#define UPPERMARGIN INT16_C(80)
#define AVGVALUE    INT16_C(1797)
#define DECALAGE    2870

//Debug values
#define NBTEST 128

#if RUSHCODE
#define INTERVAL UINT16_C(450) //on detecte des objets si on voit a +- interval autour 0
#endif


//flag variables
volatile        bool top = false,                   //drapeau à true si ñ'interruption de l'optocoupleur est déclanché
                intcapt0 = false,intcapt1 = false;  //drapeaux à true si l'interruption de chacun des capteurs est détecté
volatile TOURELLESTATE tourellestate=OFF;           //état de la tourelle (CF utils.h)
volatile TOURELLECOLOR tourellecolor = RED|BLUE;    //couleur de la tourelle (CF utils.h)


//error variable
#if ERROR
volatile ERRORVAR error=NOERROR;    //variable erreur de la tourelle (CF utils.h)
#endif

//speed variables
uint16_t fifospeed[BUFFERSPEED];         //tableau pour le registre à decalage de la vitesse


//angles variables
Points PointsOK[NBPOINTS];          //Tableau de points déjà traités
volatile uint8_t sizeOK = 0;        //Pointeur pour le tableau précedent

volatile uint16_t capt0value=0,capt1value=0;    //Angle pour chacun des capteurs (en nb de tics après le départ)


//Debug Variables
#if DEBUG
uint16_t angle1debug[NBTEST];       //Tampon pour l'angle 1 (RAW)
uint16_t angle2debug[NBTEST];       //Tampon pour l'angle 2 (RAW)
uint16_t speeddebug[NBTEST];        //Tampon pour les valeurs de vitesse (Non moyennée, c'est-à-dire, instantanée)
uint16_t anglefinaldebug[NBTEST];   //Tampon pour calibrer le décalage
#endif

//I2C buffer variable
Tosend TosendOK[NBPOINTS];          //Tampon avec les valeurs à transmettre sur l'I2c

//PWM Sinus generation
const uint8_t sinus[256]={127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,176,179,182,184,187,190,193,195,198,
                                    200,203,205,208,210,213,215,217,219,221,224,226,228,229,231,233,235,236,238,239,241,242,244,245,246,
                                    247,248,249,250,251,251,252,253,253,254,254,254,254,254,255,254,254,254,254,254,253,253,252,251,251,
                                    250,249,248,247,246,245,244,242,241,239,238,236,235,233,231,229,228,226,224,221,219,217,215,213,210,
                                    208,205,203,200,198,195,193,190,187,184,182,179,176,173,170,167,164,161,158,155,152,149,146,143,139,
                                    136,133,130,127,124,121,118,115,111,108,105,102,99,96,93,90,87,84,81,78,75,72,70,67,64,61,59,56,54,
                                    51,49,46,44,41,39,37,35,33,30,28,26,25,23,21,19,18,16,15,13,12,10,9,8,7,6,5,4,3,3,2,1,1,0,0,0,0,0,
                                    0,0,0,0,0,0,1,1,2,3,3,4,5,6,7,8,9,10,12,13,15,16,18,19,21,23,25,26,28,30,33,35,37,39,41,44,46,49,51,
                                    54,56,59,61,64,67,70,72,75,78,81,84,87,90,93,96,99,102,105,108,111,115,118,121,124};





//-----------------INTERRUPTIONS -----------------//

//Timer 0 Overflow interruption -> PWM leds
/*
Change la valeur du pwm, pour suivre une signal sinusoidale
*/
ISR(TIMER0_OVF_vect){
    static uint8_t ovfs = 0;
    ovfs++;
    /* Period - Prescale calcs
    #frequence du micro
    fosc = 16e6
    tosc = 1/fosc
    #periode de tic de l'horloge
    ttic = tosc * 64
    #periode avant le debordement du timer
    tclk = 0xFF*ttic
    tclk
    0.0010199999999999999
    #on va parcourir 256 valeurs de pwm
    256*tclk
    0.26111999999999996
    #on ajoute un diviseur ovfs pour avoir une periode plus petite
    256*tclk*8
    2.0889599999999997
    #==> Le periode de la sinusoide est de 2 s environ
    */
    static uint8_t counter=0;
    if(ovfs==8){    //prescale value
        timer0pwmModify(sinus[counter],POWTWO,tourellecolor);
        counter++;
        ovfs=0;
        //Counter goes from 255 to 0 automatically (overflow of uint8)
    }
    TIFR0 = 1<<TOV0;    //Deactivate interruption
}

//Timer 1 Overflow interruption
/*
Si le moteur de la tourelle tourne a une fréquence inférieur à 1 Hz,
on a un débordement du timer 1 et on ne peut plus calculer les angles.
*/
ISR(TIMER1_OVF_vect){
    #if ERROR
    error = TIMER1OVF;  //set error variable
    #endif
    TIFR1=1<<TOV1;      //deactivate interruption

}


//INT 0 External Interruption
/*
Interruption pour le capteur 0.
Doit garder la valeur du timer obtenu sur une variable globale (capt0value) mettre à 1 le drapeau intcapt0
*/

ISR(INT0_vect){
    uint16_t storevalue = 0;
    storevalue=TCNT1;
    intcapt0 = true;
    capt0value=storevalue;
    EIFR=1; //deactivate interruption
}

//INT 1 External Interruption
/*
Interruption pour le capteur 1.
Doit garder la valeur du timer obtenu sur une variable globale (capt1value) mettre à 1 le drapeau intcapt1
*/
ISR(INT1_vect){
    uint16_t storevalue = 0;
    storevalue=TCNT1;
    intcapt1 = true;
    capt1value=storevalue;
    EIFR=1<<1;  //deactivate interruption
}


//Timer capture interruption
/*
Déclanchée par l'opto, obtient le valeur du timer 1 et le met à 0
*/
ISR(TIMER1_CAPT_vect){
    uint16_t storevalue;
    static uint8_t pointerfifospeed=0;
    //If capture value, set capture flag and store timer difference in buffer
    timer1stop();
    storevalue=ICR1;            //store actual timer value
    TCNT1=0;
    timer1start();
    top = true;
    loadfifo(storevalue,fifospeed,BUFFERSPEED,&pointerfifospeed);

    TIFR1=1<<ICF1; //clear interrupt flag
}


//------------------------------------------------//

//-----------------I2C CALLBACK -----------------//
/*

*/
void i2cCallBack(uint8_t cmd, uint8_t dataRx[], uint8_t size){
uint8_t stderror;
switch(cmd){
    case 'I':
        /*
        Instructions de demarrage
        char couleur 'R'=Rouge,'V'=violet, 'D'=demarrage
        */
        switch(*dataRx){
            case 'R':
                tourellestate = WAITING;
                tourellecolor = RED;
                break;
            case 'V':
                tourellestate = WAITING;
                tourellecolor = BLUE;
                break;
            case 'D':
                tourellestate = STARTED;
                break;
        }
        break;

    case 'E':
        /*
        Error code asked
        char état tourelle: 'B' en fonctionnement, 'M' Moteur arrêté,
        'C' capteur sick détecte rien (pas de balise vu, ou capteur éteint)
        */
        #if ERROR
        switch(error){
            case TIMER1OVF:
                stderror = 'M';
                break;
            case NOERROR:
                stderror = 0;
            break;
            default:
            break;
        }
        #else
        stderror = 'B';
        #endif
        i2cWrite((uint8_t *)&stderror, sizeof stderror);
        break;

    case 'D':
        //send data trough i2c
        i2cWrite((uint8_t *)(TosendOK),sizeof TosendOK);
        break;
	}
}
//------------------------------------------------//

//--------------ERROR FUNCTION--------------------//
#if ERROR
void errorfunction(ERRORVAR errortype){
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        static bool errortreated =0;
        if(errortreated==false){
            switch(errortype){
                case TIMER1OVF:
                    tourellecolor = RED|BLUE;
                    ledtype(CLIGNOTER,RED|BLUE);
                    errortreated = true;
                    break;

                default:

                    break;
            }
        }

    }
}
#endif

//------------------------------------------------//

/*
TODO    -
    Use timestamp to choise last angles seen
    Optimize algorithm, paired samples
*/


int main(){

    //flag variables
    TOURELLESTATE oldstate=tourellestate;
    //Speed variables
    uint16_t speed;
    uint16_t ordered[BUFFERSPEED];
    //Angle variables
    Points PointsReg[NBREGS+2];
    uint8_t sizeReg = 0;

    int16_t capt0angle=0;
    int16_t capt1angle=0;

    //Other variables
    uint8_t i;

    //Debug variables
    #if DEBUG
    uint8_t pointerangle1debug=0;
    uint8_t pointerangle2debug=0;
    uint8_t pointerspeeddebug=0;
    uint8_t pointeranglefinaldebug=0;
    #endif


    DDRD = 1<<PD0|1<<PD1;   //debug leds or usart
    DDRD = 1<<PD7;         //Motor pwm
    DDRB = 1<<PB4|1<<PB3;   //leds pwm

    PRR = (1<<PRSPI)|(1<<PRUSART0)|(1<<PRADC);  //power reduction to disable usart, adc and spi

    timer1counterInit();
    timer0pwmInit();
    timer2pwmInit();

    setEint(0,RISINGEDGE);
    setEint(1,RISINGEDGE);

    #if RUSHCODE
    DDRC = 1<<0; //SCL as output
    #else
    i2cInit();
    #endif

    //usartTest();
    //USART_Init(103);
    sei();

    for(i=128;i<255;i++){           //Motor start sequence
        timer2pwmModify(i,POWTWO);
        _delay_ms(10);
    }
    timer1start();
    timer0start();

    //Uncomment for competition
    //while(tourellestate==OFF);

    ledtype(CLIGNOTER,tourellecolor);

    for(i=0;i<BUFFERSPEED;){      //buffer initialisation, load BUFFERSPEED values
        if(top){
            top=false;
            i++;
        }
    }
    copyandorder(fifospeed,ordered,BUFFERSPEED);
    speed = getmediane(ordered,BUFFERSPEED); //get first speed value

    while(1){

        if(top){        //top founded, calc new speed

            //Calcul de la vitesse, si on a rempli le tampon avec BUFFERSPEED valeurs
            static uint8_t counter = 0;
            counter++;
            top = false;        //clear top flag
            if(counter==BUFFERSPEED){
                counter = 0;
                copyandorder(fifospeed,ordered,BUFFERSPEED);
                speed = getmediane(ordered,BUFFERSPEED);    //speed -> time in tics needed to turn 360 deg
            }

            #if DEBUG
            loadfifo(speed,speeddebug,NBTEST,&pointerspeeddebug);
            #endif


            sizeOK = 0; //Restart SizeOK pointer
            //Calcul des valeurs des angles a envoyer

            //Algorithme pour trouver des angles similaires entre le capteur 0 et 1 -> on est en zone 2
            bool flag = false;
            while(sizeReg>1){
                flag = false;
                for(i=1;i<sizeReg;i++){
                    if( (
                        ((PointsReg[0].angle-PointsReg[i].angle)<=UPPERMARGIN)
                            &&
                        (LOWERMARGIN<=(PointsReg[0].angle-PointsReg[i].angle))
                        )
                        ||
                        (
                        ((PointsReg[0].angle-PointsReg[i].angle)<=(UPPERMARGIN+3600))
                            &&
                        ((LOWERMARGIN+3600)<=(PointsReg[0].angle-PointsReg[i].angle))
                        )
                        ||
                        (
                        ((PointsReg[0].angle-PointsReg[i].angle)<=(UPPERMARGIN-3600))
                            &&
                        ((LOWERMARGIN-3600)<=(PointsReg[0].angle-PointsReg[i].angle))
                        )
                        ){

                        //Copy register
                        PointsOK[sizeOK].captor=2;
                        PointsOK[sizeOK].angle=((PointsReg[0].angle+PointsReg[i].angle)>3600?   // angle moyenne>1800 -> first captor has the latest value
                                                (PointsReg[0].captor==3?PointsReg[0].angle:PointsReg[i].angle):
                                                (PointsReg[0].captor==1?PointsReg[0].angle:PointsReg[i].angle)); //Set the latest angle
                        sizeOK++;
                        //set checked flag
                        PointsReg[0].checked = true;
                        PointsReg[i].checked = true;
                        flag = true;
                        break;	//stop finding, we found a pair of points!
                    }
                }
                if(flag==false){	//we couldn't find the pair of points...
                    PointsOK[sizeOK]=PointsReg[0];	//copy the first point
                    //angle correction for captor 2
                    sizeOK++;
                    PointsReg[0].checked=true;
                }
                sizeReg = clearzeros(PointsReg,sizeReg);

            }
            //Copy the last one, if necessary
            if(sizeReg>0) {
                PointsOK[sizeOK]=PointsReg[0];
                sizeReg=0;  //clear the last point
                sizeOK++;
            }


            //Copier angles dans le buffer i2c
            processdata(PointsOK,TosendOK,sizeOK,NBPOINTS,DECALAGE);
            //... Data ready to be sent!
            #if DEBUG
            loadfifo(TosendOK[0].angle,anglefinaldebug,NBTEST,&pointeranglefinaldebug);
            #endif

            #if RUSHCODE
            //Mode à la rush!
            bool warning = false;
            TOURELLECOLOR oldcolor = tourellecolor;
            for(i=0;i<NBPOINTS;i++){
                if(TosendOK[i].distance==1){
                    if((TosendOK[i].angle>(3600-INTERVAL))
                       ||
                       (TosendOK[i].angle<(INTERVAL))
                       )
                            warning = true;
                }
            }
            if(warning==true) {
                PORTC |= 1<<0;
                tourellecolor = BLUE;

                if(tourellecolor!=oldcolor) ledtype(CLIGNOTER,tourellecolor);
                }
            else {
                PORTC &= ~(1<<0);
                tourellecolor = RED;
                if(tourellecolor!=oldcolor) ledtype(CLIGNOTER,tourellecolor);
            }
            #endif

        }
        if(intcapt0){
            intcapt0=false;                 //disable flag
            uint32_t capt0angle32 = (capt0value*UINT32_C(3600)) / speed;
            capt0angle = (int16_t)(capt0angle32&0xFFFF);    //calc angle
            #if DEBUG
            loadfifo(capt0angle,angle1debug,NBTEST,&pointerangle1debug);
            #endif
            PointsReg[sizeReg].captor = CAPTOR0;
            PointsReg[sizeReg].angle = capt0angle;
            PointsReg[sizeReg++].checked = false;


        }
        if(intcapt1){
            intcapt1=false;
            uint32_t capt1angle32 = (capt1value*UINT32_C(3600)) / speed;
            capt1angle = (int16_t)(capt1angle32&0xFFFF);
            #if DEBUG
            loadfifo(capt1angle,angle2debug,NBTEST,&pointerangle2debug);
            #endif
            capt1angle -= AVGVALUE;
            if(capt1angle<0) capt1angle += 3600;
            PointsReg[sizeReg].captor = CAPTOR1;
            PointsReg[sizeReg].angle = capt1angle;
            PointsReg[sizeReg++].checked = false;


        }


        if(oldstate!=tourellestate){
            oldstate = tourellestate;
            if(tourellestate==STARTED) ledtype(ON,tourellecolor);
            if(tourellestate==WAITING) ledtype(CLIGNOTER,tourellecolor);

        }

        #if ERROR
        if(error!=NOERROR){
            errorfunction(error);
        }
        #endif

    }


    return 0;
}

