/*
 * GESTIONMCP6S28.h
 *
 *  Created on: 10 may 2018
 *      Author: totof
 */

#ifndef GESTIONMCP6S28_H_
#define GESTIONMCP6S28_H_

#include "GestionMCP6S21.h"

// Classe de gestion du MCP6S2x
class GestionMCP6S28  : public GestionMCP6S21 {
public:
	enum In {
		In_0 = 0b00000000,
		In_1 = 0b00000001,
		In_2 = 0b00000010,
		In_3 = 0b00000011,
		In_4 = 0b00000100,
		In_5 = 0b00000101,
		In_6 = 0b00000110,
		In_7 = 0b00000111
	};
	
	GestionMCP6S28(SelectPin);
	void setCanal(In);
};

#endif /* GESTIONMCP6S28_H_ */
