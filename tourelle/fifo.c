#include "fifo.h"
#include <stdbool.h>


void processdata(Points * pointstoprocess, Tosend * pointsprocessed, int sizeok, int sizebuffer, int decalage){
    int i;
    int16_t saveangle;
    for(i=0;i<sizeok;i++){
        saveangle = pointstoprocess[i].angle-decalage;
        if(saveangle<0) saveangle += 3600;
        pointsprocessed[i].angle =  3600-(uint16_t)saveangle;
        pointsprocessed[i].distance = pointstoprocess[i].captor;
    }
    for(i=sizeok;i<sizebuffer;i++){
        pointsprocessed[i].distance = 0;
        pointsprocessed[i].angle=0;
    }
}

void swap(Points * a, Points *b){
	Points temp;
	temp = *a;
	*a = *b;
	*b = temp;
}


int clearzeros(Points * list,int size){
	size--;
	int nbzeros=0;
	int positionzeros[size];
	int i,j=0;

	//Zero finder!
	//Find 0 position
	for(i=0;i<=size;i++){

		if(((list+i)->checked)==true) {
			positionzeros[j++]=i;
			nbzeros++;
		}
	}


	//for(i=0;i<nbzeros;i++) printf("Zero %d position %d\n",i,positionzeros[i]);

	//clear zeros
	for(i=nbzeros-1;i>=0;i--){
		if(positionzeros[i]!=size) {
			swap(list+positionzeros[i],list+size);

		}
		size--;
	}
	//printf("Zeros Cleared!\n");
	//devuelve el nuevo size del vector
	return size+1;


}

void loadfifo(uint16_t value, uint16_t * fifo, uint8_t lenlista, uint8_t * pointer){

	//Copy value on top of the fifo
	if((*pointer)==lenlista) *pointer=0;
	*(fifo+*pointer)=value;
	(*pointer)++;

}

void clearfifo(uint16_t * fifo, uint8_t lenlista){
    uint8_t counter = 0;
    while(counter<lenlista) {
        counter++;
        *(fifo++) = 0;
    }

}
void bubbleSort(volatile unsigned int numbers[], int array_size){
  int i,j,temp;
  for (i =(array_size-1); i>0; i--)
  {
    for (j = 1; j <= i; j++)
    {
      if (numbers[j-1] > numbers[j])
      {
        temp = numbers[j-1];
        numbers[j-1] = numbers[j];
        numbers[j] = temp;
      }
    }
  }
}


void echanger(unsigned int tableau[], unsigned int a,unsigned int b){
    int temp = tableau[a];
    tableau[a] = tableau[b];
    tableau[b] = temp;
}

void quickSort(unsigned int tableau[], int debut, int fin){
    int gauche = debut-1;
    int droite = fin+1;
    const unsigned int pivot = tableau[debut];

    /* Si le tableau est de longueur nulle, il n'y a rien à faire. */
    if(debut >= fin)
        return;

    /* Sinon, on parcourt le tableau, une fois de droite à gauche, et une
       autre de gauche à droite, à la recherche d'éléments mal placés,
       que l'on permute. Si les deux parcours se croisent, on arrête. */
    while(1){
        do droite--; while(tableau[droite] > pivot);
        do gauche++; while(tableau[gauche] < pivot);

        if(gauche < droite)
            echanger(tableau, gauche, droite);
        else break;
    }

    /* Maintenant, tous les éléments inférieurs au pivot sont avant ceux
       supérieurs au pivot. On a donc deux groupes de cases à trier. On utilise
       pour cela... la méthode quickSort elle-même ! */
    quickSort(tableau, debut, droite);
    quickSort(tableau, droite+1, fin);
}



void copyandorder(unsigned int * fifo,unsigned int * ordened, int listsize){
	int j;

	//copy fifo list in ordered list

	unsigned int * glissant=ordened;
	for(j=0;j<listsize;j++) *(glissant++)=*(fifo++);
	//bubbleSort(ordened,lenlista);
	quickSort(ordened,0,listsize-1);


}

unsigned int getmediane(uint16_t * list, uint8_t listsize){
    //get the middle of the list and divide in 2
    //EXACT only if listize is a power of 2
	if(listsize<2) return list[0];
	else return ((list[(listsize>>2)-1]+list[(listsize>>2)])>>1);
}
