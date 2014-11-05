#include <avr/io.h>

void USART_Init(unsigned int ubrr){
    UBRR0H = (unsigned char)(ubrr>>8)&0xF;
    UBRR0L = (unsigned char)(ubrr);
    UCSR0B = (0<<RXEN0)|(1<<TXEN0);    //only send!
    //8 data, 1 stop bits
    UCSR0C = (0<<UMSEL00)|(0<<UPM00)|(0<<USBS0)|(3<<UCSZ00);

}

void USART_Transmit(unsigned char data){
    while ( !( UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

void USART_SendChain(unsigned char * chain){
    while(*chain){
        USART_Transmit(*chain++);
    }
}

void modeloatoi(unsigned char * cadena, unsigned int value)
{

char textoinvertido[5]; //usada para almacenar los caracteres del num, en forma invertida
int i=0,j; //contadores
unsigned int ValorDividido=value; //valor a convertir

	do{
		/*
		 * Algoritmo para cambiar de base por divisiones sucesivas
		 * VER http://es.wikipedia.org/wiki/Sistema_binario
		 * en la parte de Conversión entre binario y decimal,
		 * solo que aqui nuestro numero hay que pasarlo por divisiones a
		 * 10
		 *
		 */
		textoinvertido[i]=(char)(ValorDividido%10)+48;
		ValorDividido=ValorDividido/10;
		i++;} while (ValorDividido);
	/*
	 * la condicion de escape del bucle es mientras el valor dividido no sea nulo
	 */
	j = i;
	while (i){
		/*
		 * Bucle para copiar el texto invertido a la cadena
		 * pasada a la funcion
		 */
		cadena[j-i]=textoinvertido[i-1];
		i--;
	}
	cadena[j]='\0'; //Agregado del cero terminal
}


void USART_SendU16(unsigned int number){
    unsigned char chain[6];
    modeloatoi(chain,number);
    USART_SendChain(chain);

}

void USART_SendU32(uint32_t number){
    unsigned char chain[11];
    modeloatoi(chain,(unsigned int) number);
    modeloatoi(chain+5,(unsigned int)(number>>16));
    USART_SendChain(chain);
}
