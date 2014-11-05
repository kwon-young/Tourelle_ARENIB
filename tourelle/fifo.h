#include <stdbool.h>
#include <stdint.h>
typedef struct _points{
	uint8_t captor;
	int16_t angle;
	bool checked;
}Points;

typedef struct _tosend{
    uint8_t distance;
    uint16_t angle;
}Tosend;


void processdata(Points *, Tosend *, int , int, int);
void swap(Points * a, Points *b);
int clearzeros(Points * list,int size);
void loadfifo(unsigned int , unsigned int * , uint8_t, uint8_t *);
void copyandorder(unsigned int * , unsigned int * , int );
unsigned int getmediane(uint16_t * list, uint8_t listsize);
void clearfifo(unsigned int * fifo, uint8_t lenlista);
