/*
 * GestionMCP6S21.cpp
 *
 *  Created on: 10 may 2018
 *      Author: totof
 * Controle un MCP6S21 en SPI avec la librairie wiringPi sur Raspberry Pi
 */
 
#include <SPI.h>
#include "GestionMCP6S21.h"

// *****************
// Constructeur vide
// *****************
GestionMCP6S21::GestionMCP6S21() {
}

// ***************************************
// Constructeur
// @param la broche de sélection
// Mémorise la broche et initialise le SPI
// ***************************************
GestionMCP6S21::GestionMCP6S21(SelectPin pSelectPin) {
	init(pSelectPin);
}

// **************************************
// Initialisation
// @param pChannel le channel
// Mémorise le canal et initialise le SPI
// **************************************
void GestionMCP6S21::init(SelectPin pSelectPin) {
	selectPin = pSelectPin;
	pinMode(selectPin, OUTPUT);
	// initialize SPI:
	SPI.begin();
}

// ****************
// Arrête le module
// ****************
void GestionMCP6S21::shutdown(void) {
	write(SHUTDOWN, 0x00);
}

// *****************
// Paramètre le gain
// @Param le gain
// *****************
void GestionMCP6S21::setGain(Gain gain) {
	write(GAIN, gain);
}

// ******************
// Paramètre l'entrée
// @Param l'entrée
// ******************
void GestionMCP6S21::setCanal(In in) {
	write(CANAL, in);
}

// ****************************
// Ecrit 16 bits dans le module
// @Param l'action
// @Param la valeur
// ****************************
void GestionMCP6S21::write(char val0, char val1) {
	digitalWrite(selectPin, LOW);
	delay(100);
	//  send in the address and value via SPI:
	SPI.transfer(val0);
	SPI.transfer(val1);
	delay(100);
	// take the SS pin high to de-select the chip:
	digitalWrite(selectPin, HIGH);
}

// ***********
// Destructeur
// ***********
GestionMCP6S21::~GestionMCP6S21() {
	pinMode(selectPin, INPUT);
}



