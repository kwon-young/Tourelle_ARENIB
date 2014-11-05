/*Fichier conçue pour la génération de la documentation automatique avec
* Doxygen.
*/
//! \file **********************************************************************
//!
//! \brief Ce fichier contiens le module permettent de manipuler l'i2c en esclave avec le twi pour l'arenib.
//!
//! - Compilateur		            : GCC-AVR
//! - Composent supporter   : AVR8 supportent le TWI (tester sur un atmega644).
//!
//!
//! \author Maleyrie Antoine
//! \version 0.2
//! \date 21 Décembre 2011
//!
//! ****************************************************************************

//! \defgroup i2c
//! Rapidement :
//!	Il faut définir l'adresse du composent via la define I2C_ADD qui doit ce trouver dans config.h il faut aussi indiquer la fonction de call back via la define I2C_CALL_BACK. Et enfin il faut aussi définir la taille des buffers de lecture et d'écriture avec I2C_SIZE_BUFFER_RX et I2C_SIZE_BUFFER_TX.
//!	exemple ->
//! \code
//! #define I2C_ADD 0x51
//! #define I2C_CALL_BACK i2cCallBack
//! #define I2C_SIZE_BUFFER_RX 32
//! #define I2C_SIZE_BUFFER_TX 32
//! \endcode
//!
//! La fonction de call back doit avoirs le prototype suivent :
//! \code
//! void i2cCallBack(uint8_t cmd, uint8_t dataRx[], uint8_t size);
//! \endcode
//!
//! Puis, initialiser la bibliothèque via la fonction \ref i2cInit(), vous pouvez utiliser \ref i2cWrite() dans la fonction de call back pour répondre au maître. \n
//!
//!	Un exemple d'implémentation pour la fonction de call back est données si dessous.
//!
//! \code
//!void i2cCallBack(uint8_t cmd, uint8_t dataRx[], uint8_t size)
//!{
//!	static int16_t test = 0;
//!	switch(cmd)
//!	{
//!		case 1:
//!			test++;
//!			i2cWrite((uint8_t *)&test, sizeof test);
//!		break;
//!	}
//!}
//! \endcode
//!
//! @{

#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED

//_____ I N C L U D E S ________________________________________________________
#include <stdbool.h>
#include "i2c_config.h"

//_____ M A C R O S ____________________________________________________________
// Vérifie l'existiviter de la fonction de callBack
#if !defined(I2C_CALL_BACK)
#error I2C_CALL_BACK is no defined for i2c.h.
#endif

// Vérifie l'existiviter de la define représentant l'adresse.
#if !defined(I2C_ADD)
#error I2C_ADD is no defined for i2c.h.
#endif

// Vérifie l'existiviter de la define représentant la taille du buffer de lecture.
#if !defined(I2C_SIZE_BUFFER_RX)
#error I2C_SIZE_BUFFER_RX is no defined for i2c.h.
#endif

// Vérifie l'existiviter de la define représentant la taille du buffer d'écriture.
#if !defined(I2C_SIZE_BUFFER_RX)
#error I2C_SIZE_BUFFER_TX is no defined for i2c.h.
#endif

//_____ E N U M ________________________________________________________________

//_____ P R O T O T Y P E S ____________________________________________________
//! \brief Initialisation de l'i2c.
void i2cInit();
//void i2cInit(const uint8_t addres, const bool allCall);

//! \brief Permet d'écrire des données.
//!
//! \note Cette fonction ne vérifie pas la validité du nombre de données à écrire. Vous devez être sur que le paramètre \p size est inférieur ou égale à la maccro \b I2C_SIZE_BUFFER_TX.
//! \param data sont les données à envoyer au maître.
//! \param size est le nombre de données.
void i2cWrite(uint8_t data[], const uint8_t size);

//! \brief Permet d'activer le'i2c.
//! \note L'i2c est déjà activer à l'appelle de \ref i2cInit().
//! \see i2c_disable()
static inline void i2c_enable()
{
	TWCR |= _BV(TWEN) | _BV(TWIE);
}

//! \brief Permet de désactiver l'i2c.
//! \see i2c_enable()
static inline void i2c_disable()
{
	TWCR &= ~(_BV(TWEN) & _BV(TWIE));
}

//! @}

#endif // I2C_H_INCLUDED
