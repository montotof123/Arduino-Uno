/*
 * GestionMCP6S28.cpp
 *
 *  Created on: 10 may 2018
 *      Author: totof
 * Controle un MCP6S28 en SPI avec la librairie wiringPi sur Raspberry Pi
 */
 
#include "GestionMCP6S28.h"

// ***************************************
// Constructeur
// @param la broche de sélection
// Mémorise la broche et initialise le SPI
// ***************************************
GestionMCP6S28::GestionMCP6S28(SelectPin pSelectPin) {
	init(pSelectPin);
}

// ******************
// Paramètre l'entrée
// @Param l'entrée
// ******************
void GestionMCP6S28::setCanal(In in) {
	write(CANAL, in);
}

