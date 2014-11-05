/*Fichier conçue pour la génération de la documentation automatique avec
* Doxygen.
*/
//! \file **********************************************************************
//!
//! \brief Ce fichier contiens le module permettent de manipuler l'i2c en esclave avec le twi pour l'arenib.
//!
//! - Compilateur		    : GCC-AVR
//! - Composent supporter   : AVR8 supportent le TWI (tester sur un atmega644).
//!
//!
//! \author Maleyrie Antoine
//! \version 0.2
//! \date 21 Décembre 2011
//!
//! ****************************************************************************


//_____ I N C L U D E S ________________________________________________________
#include <avr/interrupt.h>
#include <util/twi.h>

#include "i2c.h"
//_____ M A C R O S ____________________________________________________________

//_____ P R O T O T Y P E S ____________________________________________________
extern void I2C_CALL_BACK(uint8_t cmd, uint8_t dataRx[], uint8_t size);

//_____ I M P L E M E N T A T I O N S __________________________________________

// Buffer pour la lecture.
uint8_t _i2cDataRx[I2C_SIZE_BUFFER_RX+1];	//+1 pour la commende.

// Buffer pour l'écriture.
uint8_t _i2cDataTx[I2C_SIZE_BUFFER_TX];
uint8_t _i2cSizeTx = 0;

void i2cInit()
{
	//Affect l'adresse au composent.
	TWAR = I2C_ADD<<1;
	//Active la reconnaissance générale.
//	if(allCall)
//		TWAR |= _BV(TWGCE);

	//Active l'i2c et prépare le composent à écrire le bit ack une foi reconnue.
	TWCR = _BV(TWINT)|_BV(TWEA)|_BV(TWEN) | _BV(TWIE);
}

void i2cWrite(uint8_t data[], const uint8_t size)
{
	//recopie du buffer pour l'écriture.
	for(uint8_t i = 0; i != size; i++)
		_i2cDataTx[i] = data[i];

	_i2cSizeTx = size;
}

ISR(TWI_vect)
{
	//L'index du tableau des données.
	static uint8_t indexData = 0;

	switch(TW_STATUS)
	{
		case TW_ST_ARB_LOST_SLA_ACK:	// Le composent c'est reconnu en mode écriture et le bit ack a été reçu.
		case TW_ST_SLA_ACK:				// Le composent c'est reconnu en mode écriture et le bit ack a été reçu.
			// Mise à 0 de l'index.
			indexData = 0;
		case TW_ST_DATA_ACK:			// Une donnée a été envoyer et le bit ack a été reçu.
			// La donnée a envoie.
			TWDR = _i2cDataTx[indexData];

			//Incrément l'index.
			indexData++;

			// Vérifie la taille du buffer d'écriture.
			if(indexData < _i2cSizeTx)
				// Envoi d'une données. Puis recevoir le bit ack.
				TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
			else
				// Envoi d'une données. Puis recevoir le bit nack.
				TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
		break;
		case TW_ST_DATA_NACK:			// Une donnée a été envoyer et le bit nack a été reçu.
		case TW_ST_LAST_DATA:			// La dernière donnée a été envoyer.
			//Active l'i2c et prépare le composent à écrire le bit ack une foi reconnue.
			TWCR = _BV(TWINT)|_BV(TWEA)|_BV(TWEN) | _BV(TWIE);
		break;
//		case TW_SR_ARB_LOST_SLA_ACK:
//		case TW_SR_ARB_LOST_GCALL_ACK:
		case TW_SR_SLA_ACK:				// Le composent c'est reconnu en mode lecture.
		case TW_SR_GCALL_ACK:			// Le composent c'est reconnu en mode lecture.
			// Mise à 0 de l'index.
			indexData = 0;

			// Vérifie la taille du buffer de lecture.
			#if I2C_SIZE_BUFFER_RX != 0
				// Un octet va être ressue et le bit ack va être transmis.
				TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
			#else
				// Un octet va être ressue et le bit nack va être transmis.
				TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
			#endif
		break;
		case TW_SR_GCALL_DATA_ACK:		// Une données a été reçus, le bit ask a été transmis.
		case TW_SR_DATA_ACK:			// Une données a été reçus, le bit ask a été transmis.
			//Récupère l'octet ressue.
			_i2cDataRx[indexData] = TWDR;

			//Incrément l'index.
			indexData++;

			// Vérifie la taille du buffer de lecture.
			if(I2C_SIZE_BUFFER_RX+1 > indexData)
				// Un octet va être ressue et le bit ack va être transmis.
				TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
			else
				// Un octet va être ressue et le bit nack va être transmis.
				TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
		break;
		case TW_SR_STOP:				// La condition de stop ou de répète start a été reçus.
			//Active l'i2c et prépare le composent à écrire le bit ack une foi reconnue.
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);

			//Appelle de la fonction de call back.
			I2C_CALL_BACK(_i2cDataRx[0], _i2cDataRx+1, indexData-1);
		break;
		case TW_SR_GCALL_DATA_NACK:		// Une données a été reçus, le bit nask a été transmis.
		case TW_SR_DATA_NACK:			// Une données a été reçus, le bit nask a été transmis.
//			//Récupère l'octet ressue.
//			_i2cDataRx[indexData] = TWDR;
//
//			//Incrément l'index.
//			indexData++;
//
//			//Active l'i2c et prépare le composent à écrire le bit ack une foi reconnue.
//			TWCR = _BV(TWINT)|_BV(TWEA)|_BV(TWEN) | _BV(TWIE);
//
//			//Appelle de la fonction de call back.
//			I2C_CALL_BACK(_i2cDataRx[0], _i2cDataRx+1, indexData-1);
//		break;
		case TW_BUS_ERROR:
		case TW_NO_INFO:
		default:
			//Active l'i2c et prépare le composent à écrire le bit ack une foi reconnue.
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
		break;
	}
}
