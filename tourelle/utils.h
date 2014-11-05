
//Data types boolean
#include <stdbool.h>
#define USARTFUNCTIONS 0    //functions pour activer l'usart

typedef enum{NOERROR,TIMER1OVF,NODETECTION} ERRORVAR;   //types d'erreur de la tourelle

typedef enum{OFF,WAITING,STARTED} TOURELLESTATE;        //etats de la tourelle

typedef unsigned char TOURELLECOLOR;                    //couleurs de la tourelle
#define RED 1<<0
#define BLUE 1<<1

typedef enum{CLIGNOTER,ON} LEDSTATE;                    //etat de led de la tourelle

void ledtype(LEDSTATE ledstate,TOURELLECOLOR ledclr);
void clignoter(unsigned char led,unsigned int delay);
void afichercounter();
void pwmtest();

#if USARTFUNCTIONS
void usartTest();
void usartNumberTest();
#endif
