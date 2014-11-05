//For asynch Normal Mode
#define BAUD 9600
#define UBRRVALUE (F_CPU/(16*BAUD))-1
#define U2XVALUE 0
void USART_Init(unsigned int ubrr);
void USART_Transmit(unsigned char data);
void USART_SendChain(char * chain);
void USART_SendU16(unsigned int number);
void USART_SendU32(uint32_t number);
