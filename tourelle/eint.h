

typedef unsigned char InterruptPin;
typedef unsigned char Mode;
//Flags for ISCn0
#define LEVEL       0
#define ANY         1
#define FALLINGEDGE 2
#define RISINGEDGE  3

int setEint(InterruptPin,Mode);
int clearEint(InterruptPin);
