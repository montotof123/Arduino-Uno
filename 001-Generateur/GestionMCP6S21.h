/*
 * GESTIONMCP6S21.h
 *
 *  Created on: 27 may 2018
 *      Author: totof
 *  Arduino Uno
 */

#ifndef GESTIONMCP6S21_H_
#define GESTIONMCP6S21_H_

#include "GestionSPI.h"

// Classe de gestion du MCP6S21
class GestionMCP6S21 {
public:
	enum Register {
		SHUTDOWN = 0b00100000,
		GAIN =     0b01000000,
		CANAL   =  0b01000001
	};

	enum Gain {
		Gain_1 =  0b00000000,
		Gain_2 =  0b00000001,
		Gain_4 =  0b00000010,
		Gain_5 =  0b00000011,
		Gain_8 =  0b00000100,
		Gain_10 = 0b00000101,
		Gain_16 = 0b00000110,
		Gain_32 = 0b00000111
	};

	enum In {
		In_0 = 0b00000000
	};

	GestionMCP6S21();
	GestionMCP6S21(SelectPin);
	void init(SelectPin);
	
	void shutdown(void);
	void setGain(Gain);
	void setCanal(In);

  ~GestionMCP6S21();

protected:
	void write(char, char);

private:
	SelectPin selectPin;
};



#endif /* GESTIONMCP6S21_H_ */
