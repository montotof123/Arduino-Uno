// *********************
// Generateur de signaux
// totof 2018/07/28
// *********************

#include <Wire.h>
#include <LiquidCrystal.h>
#include "Adafruit_MPR121.h"
#include "AD9833.h"
#include "GestionMCP6S28.h"
#include "GestionSPI.h"
#include "DigiPotX9Cxxx.h"

Adafruit_MPR121 cap = Adafruit_MPR121();
AD9833 gen(PD_2);  
GestionMCP6S28 aop(PB_2);
LiquidCrystal lcd(8, 9, 14, 15, 16, 17);
DigiPot luminosite(6, 7, 3);
DigiPot contraste(4, 5, 1);

// Controle des touches du MCP121
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

// Signal actif/inactif
bool signal;

// Sinus, triangle, carre
uint16_t typeSignal; 

// Réglage MCP6S28
uint8_t indiceTension; // 0.6V, 1.0V, 1.6V, 1.9V, 2.4V, 3.2V, 3.8V, 4.8V
const uint8_t canalSinusTriangle[8] = {aop.In_0, aop.In_1, aop.In_1, aop.In_2, aop.In_0, aop.In_1, aop.In_2, aop.In_0};
const uint8_t gainSinusTriangle[8] = {aop.Gain_1, aop.Gain_5, aop.Gain_8, aop.Gain_16, aop.Gain_4, aop.Gain_16, aop.Gain_32, aop.Gain_8};
const uint8_t canalCarre[8] = {aop.In_3, aop.In_4, aop.In_1, aop.In_2, aop.In_3, aop.In_1, aop.In_2, aop.In_0};
const uint8_t gainCarre[8] = {aop.Gain_1, aop.Gain_2, aop.Gain_1, aop.Gain_2, aop.Gain_4, aop.Gain_2, aop.Gain_4, aop.Gain_1};

// Frequence par son tableau de chiffre
uint8_t frequence[10];

// Frequence en Hz (recomposition du tableau de chiffre)
uint16_t frequenceHz;

// Position du curseur
uint8_t cursor;

// Luminosite
uint8_t valLum;

// Contraste
uint8_t valContraste;

// *****************************************
//       ***** ***** ***** *   * *****
//       *     *       *   *   * *   *
//       ***** ****    *   *   * *****
//           * *       *   *   * *
//       ***** *****   *   ***** *
// *****************************************
void setup() {
	// Debug ON
	Serial.begin(115200);
	// Debug OFF
	
	// Valeurs initiales
	// frequence 2000Hz
	frequence[0] = 0;
	frequence[1] = 0;
	frequence[2] = 0; // point
	frequence[3] = 0;
	frequence[4] = 0;
	frequence[5] = 2;
	frequence[6] = 0; // point
	frequence[7] = 0;
	frequence[8] = 0;
	frequence[9] = 0;
	
	// Signal inactif
	signal = false;
	
	// Signal triangulaire
	typeSignal = TRIANGLE_WAVE; 
	
	// Tension 0.6V
	indiceTension = 0;

	// Curseur a gauche
	cursor = 0;

	// Luminosite moyenne
	valLum = 1;

	// Contraste moyen
	valContraste = 3;
	
	// Debug ON
	Serial.println("Init valeur par défaut");
	// Debug OFF
	
	// Initialisation circuits
	// Initialisation générateur
	gen.Begin();
	gen.EnableOutput(signal);
	setFrequence();
	// Debug ON
	Serial.println("Init générateur");
	// Debug OFF
	
	// Initialisation amplificateur
	setAmplificateur();
	// Debug ON
	Serial.println("Init amplificateur");
	// Debug OFF

	// Initialisation afficheur
	lcd.begin(16, 2);
	lcd.setCursor(0, 0);
    lcd.print("Generateur V1.00");
	luminosite.set(valLum);
	contraste.set(valContraste);
 	// Debug ON
	Serial.println("Init afficheur");
 	// Debug OFF

	// Initialisation touches sensitives
	lcd.setCursor(0, 1);
	lcd.print("MPR121 not found"); // Message d'erreur si plantage begin
	if (!cap.begin(0x5A)) {
		// Une erreur dans le begin plante le programme
		// Donc, j'ai laissé cette partie pour le cote 
		//  ludique, mais elle ne sert a rien

		while (1);
	}
	// Debug ON
	Serial.println("Init clavier");
	// Debug OFF

	// Message initialisation OK
	lcd.setCursor(0, 1);
	lcd.print("    Init. OK    ");
	delay(4000);
	// Debug ON
	Serial.println("Init fin");
	// Debug OFF
}

// ****************************************
//         *     ***** ***** *****
//         *     *   * *   * *   *
//         *     *   * *   * *****
//         *     *   * *   * *
//         ***** ***** ***** *
// ****************************************

void loop() {
	// Touches actives
	currtouched = cap.touched();
  
    // Balayage des touches de 0 à 11
	for (uint8_t i=0; i<12; i++) {
		// Touche active et non active avant
		if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
			switch(i) {
				// Type de signal Sinus, triangle ou carre
				case 0 : changeSignal();
						 break;
				// Niveau Signal +
				case 1 : levelUp();
						 break;
				// Niveau Signal -
				case 2 : levelDown();
						 break;
				// Signal On/Off
				case 3 : onOff();
						 break;
				// Luminosite afficheur +
				case 4 : lumUp();
						 break;
				// Luminosite afficheur -
				case 5 : lumDown();
						 break;
				// Case frequence gauche
				case 6 : cursorLeft();
						 break;
				// Case frequence droite
				case 7 : cursorRight();
						 break;
				// Frequence +
				case 8 : freqUp();
						 break;
				// frequence -
				case 9 : freqDown();
						 break;
				// Contraste +
				case 10: contrastUp();
						 break;
				// Contraste -
				case 11: contrastDown();
						 break;
			}
		}
	}

	// Reset etat touche
	lasttouched = currtouched;
	
	// Affichage lcd
	// Type de signal
	lcd.noCursor();
	lcd.setCursor(0, 0);
	switch(typeSignal) {
		case SINE_WAVE    : lcd.print("Sinus       "); break;
		case TRIANGLE_WAVE: lcd.print("Triangle    "); break;
		case SQUARE_WAVE  : lcd.print("Carre       "); break;
	}
	
	// Tension
	lcd.setCursor(12, 0);
	switch(indiceTension) {
		case 0: lcd.print("0.6V"); break;
		case 1: lcd.print("1.0V"); break;
		case 2: lcd.print("1.6V"); break;
		case 3: lcd.print("1.9V"); break;
		case 4: lcd.print("2.4V"); break;
		case 5: lcd.print("3.2V"); break;
		case 6: lcd.print("3.8V"); break;
		case 7: lcd.print("4.8V"); break;
	}
	
	// Frequence
	lcd.setCursor(0, 1);
    lcd.print(frequence[0]);
	lcd.setCursor(1, 1);
    lcd.print(frequence[1]);
	lcd.setCursor(2, 1);
    lcd.print(".");
	lcd.setCursor(3, 1);
    lcd.print(frequence[3]);
	lcd.setCursor(4, 1);
    lcd.print(frequence[4]);
	lcd.setCursor(5, 1);
    lcd.print(frequence[5]);
	lcd.setCursor(6, 1);
    lcd.print(".");
	lcd.setCursor(7, 1);
    lcd.print(frequence[7]);
	lcd.setCursor(8, 1);
    lcd.print(frequence[8]);
	lcd.setCursor(9, 1);
    lcd.print(frequence[9]);
	lcd.setCursor(10, 1);
    lcd.print("Hz ");
	
	// Actif
	lcd.setCursor(13, 1);
	if(signal) {
		lcd.print(" ON");
	} else {
		lcd.print("OFF");
	}
	
	// Position du curseur
	lcd.cursor();
	lcd.setCursor(cursor, 1);
	
	// Temporisation
	delay(100);
}

// ****************************
// Change le type de signal
// Sinus --> triangle --> carre
// ^                          |
// |                          v
// <---------------------------
// **********************************
void changeSignal(void) {
	// Debug ON
	Serial.println("--> changeSignal");
	// Debug OFF
	
	switch(typeSignal) {
		case SINE_WAVE: typeSignal = TRIANGLE_WAVE; 
				        gen.ApplySignal(TRIANGLE_WAVE, REG0, frequenceHz);
				        break;
		case TRIANGLE_WAVE: typeSignal = SQUARE_WAVE; 
				            gen.ApplySignal(SQUARE_WAVE, REG0, frequenceHz);
				            break;
		case SQUARE_WAVE: typeSignal = SINE_WAVE; 
				          gen.ApplySignal(SINE_WAVE, REG0, frequenceHz);
				          break;
	}
	setAmplificateur();
	
	// Debug ON
	Serial.print(" Valeur signal: ");
	Serial.println(typeSignal);
	Serial.println("<-- changeSignal");
	// Debug OFF
}

// ***************************************
// Augmente le gain de l'ampli si possible
// ***************************************
void levelUp(void) {
	// Debug ON
	Serial.println("--> levelUp");
	// Debug OFF

	switch(indiceTension) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6: indiceTension++; 
				setAmplificateur();
				break;
		case 7: break; // Tension max 
	}
	
	// Debug ON
	Serial.print(" Valeur indiceTension: ");
	Serial.println(indiceTension);
	Serial.println("<-- levelUp");
	// Debug OFF
}

// *************************************
// Baisse le gain de l'ampli si possible
// *************************************
void levelDown(void){
	// Debug ON
	Serial.println("--> levelDown");
	// Debug OFF

	switch(indiceTension) {
		case 0: break; // Tension min
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7: indiceTension--; 
				setAmplificateur();
				break;
	}
	
	// Debug ON
	Serial.print(" Valeur indiceTension: ");
	Serial.println(indiceTension);
	Serial.println("<-- levelDown");
	// Debug OFF
}

// *******************************
// Bascule le signal Actif/Inactif
// *******************************
void onOff(void){
	// Debug ON
	Serial.println("--> onOff");
	// Debug OFF

	signal = !signal;
	setAmplificateur();
	gen.EnableOutput(signal);
	
	// Debug ON
	Serial.print(" Valeur signal: ");
	Serial.println(signal);
	Serial.println("<-- onOff");
	// Debug OFF
}


// *************************************
// Augmente la luminosite de l'afficheur
// *************************************
void lumDown(void){
	// Debug ON
	Serial.println("--> lumDown");
	// Debug OFF

	if(valLum < 100) {
		valLum++;
		luminosite.set(valLum);
	}
	
	// Debug ON
	Serial.print(" Valeur valLum: ");
	Serial.println(valLum);
	Serial.print(" Valeur X9C103P: ");
	Serial.println(luminosite.get());
	Serial.println("<-- lumDown");
	// Debug OFF
}

// ***********************************
// Baisse la luminosite de l'afficheur
// ***********************************
void lumUp(void){
	// Debug ON
	Serial.println("--> lumUp");
	// Debug OFF

	if(valLum > 0) {
		valLum--;
		luminosite.set(valLum);
	}
	
	// Debug ON
	Serial.print(" Valeur valLum: ");
	Serial.println(valLum);
	Serial.print(" Valeur X9C103P: ");
	Serial.println(luminosite.get());
	Serial.println("<-- lumUp");
	// Debug OFF
}

// *****************************************************
// Curseur de l'afficheur de la frequence vers la gauche
// *****************************************************
void cursorLeft(void){
	// Debug ON
	Serial.println("--> cursorLeft");
	// Debug OFF

	switch(cursor) {
		case 0: break; // Minimum
		case 1: cursor = 0; break;
		case 3: cursor = 1; break;
		case 4: cursor = 3; break;
		case 5: cursor = 4; break;
		case 7: cursor = 5; break;
		case 8: cursor = 7; break;
		case 9: cursor = 8; break;
	}
	
	// Debug ON
	Serial.print(" Valeur cursor: ");
	Serial.println(cursor);
	Serial.println("<-- cursorLeft");
	// Debug OFF
}

// *****************************************************
// Curseur de l'afficheur de la frequence vers la droite
// *****************************************************
void cursorRight(void){
	// Debug ON
	Serial.println("--> cursorRight");
	// Debug OFF

	switch(cursor) {
		case 0: cursor = 1; break;
		case 1: cursor = 3; break;
		case 3: cursor = 4; break;
		case 4: cursor = 5; break;
		case 5: cursor = 7; break;
		case 7: cursor = 8; break;
		case 8: cursor = 9; break;
		case 9: break; // Maximum
	}
	
	// Debug ON
	Serial.print(" Valeur cursor: ");
	Serial.println(cursor);
	Serial.println("<-- cursorRight");
	// Debug OFF
}

// *****************************************************
// Augmente le chiffre de la frequence ou est le curseur
// *****************************************************
void freqUp(void){
	// Debug ON
	Serial.println("--> freqUp");
	// Debug OFF

	switch(cursor) {
		// Frequence max 25.000.000 Hz (25MHz)
		// Dizaine de million max  a 2
		case 0: if(frequence[0] < 2) {
					frequence[0] = frequence[0] + 1;
				}
				if(frequence[0] == 2 && frequence[1] > 5) {
					frequence[1] = 5;
				}
				break;
		// Million max a 5 si dizaine de million max a 2
		case 1: if(frequence[1] < 9) {
					frequence[1] = frequence[1] + 1;
				}
				if(frequence[0] == 2 && frequence[1] > 5) {
					frequence[1] = 5;
				}
				break;
		case 3: if(frequence[3] < 9) {
					frequence[3] = frequence[3] + 1;
				}
				break;
		case 4: if(frequence[4] < 9) {
					frequence[4] = frequence[4] + 1;
				}
				break;
		case 5: if(frequence[5] < 9) {
					frequence[5] = frequence[5] + 1;
				}
				break;
		case 7: if(frequence[7] < 9) {
					frequence[7] = frequence[7] + 1;
				}
				break;
		case 8: if(frequence[8] < 9) {
					frequence[8] = frequence[8] + 1;
				}
				break;
		case 9: if(frequence[9] < 9) {
					frequence[9] = frequence[9] + 1;
				}
				break;
	}
	setFrequence();
	
	// Debug ON
	Serial.print(" Valeur frequenceHz: ");
	Serial.println(frequenceHz);
	Serial.println("<-- freqUp");
	// Debug OFF
}

// ***************************************************
// Baisse le chiffre de la frequence ou est le curseur
// ***************************************************
void freqDown(void){
	// Debug ON
	Serial.println("--> freqDown");
	// Debug OFF

	switch(cursor) {
		case 0: if(frequence[0] > 0) {
					frequence[0] = frequence[0] - 1;
				}
				break;
		case 1: if(frequence[1] > 0) {
					frequence[1] = frequence[1] - 1;
				}
				break;
		case 3: if(frequence[3] > 0) {
					frequence[3] = frequence[3] - 1;
				}
				break;
		case 4: if(frequence[4] > 0) {
					frequence[4] = frequence[4] - 1;
				}
				break;
		case 5: if(frequence[5] > 0) {
					frequence[5] = frequence[5] - 1;
				}
				break;
		case 7: if(frequence[7] > 0) {
					frequence[7] = frequence[7] - 1;
				}
				break;
		case 8: if(frequence[8] > 0) {
					frequence[8] = frequence[8] - 1;
				}
				break;
		case 9: if(frequence[9] > 0) {
					frequence[9] = frequence[9] - 1;
				}
				break;
	}
	setFrequence();
	
	// Debug ON
	Serial.print(" Valeur frequenceHz: ");
	Serial.println(frequenceHz);
	Serial.println("<-- freqDown");
	// Debug OFF
}

// ************************************
// Augmente le contraste de l'afficheur
// ************************************
void contrastDown(void){
	// Debug ON
	Serial.println("--> contrastDown");
	// Debug OFF

	if(valContraste < 100) {
		valContraste++;
		contraste.set(valContraste);
	}
	
	// Debug ON
	Serial.print(" Valeur valContraste: ");
	Serial.println(valContraste);
	Serial.print(" Valeur X9C103P: ");
	Serial.println(contraste.get());
	Serial.println("<-- contrastDown");
	// Debug OFF
}

// **********************************
// Baisse le contraste de l'afficheur
// **********************************
void contrastUp(void){
	// Debug ON
	Serial.println("--> contrastUp");
	// Debug OFF

	if(valContraste > 0) {
		valContraste--;
		contraste.set(valContraste);
	}
	
	// Debug ON
	Serial.print(" Valeur valContraste: ");
	Serial.println(valContraste);
	Serial.print(" Valeur X9C103P: ");
	Serial.println(contraste.get());
	Serial.println("<-- contrastUp");
	// Debug OFF
}

// *****************************************************************
// Converti le tableau de chiffre de la frequence en frequence en Hz 
// *****************************************************************
void setFrequence(void) {
	// Debug ON
	Serial.println("   --> setFrequence");
	// Debug OFF

	// frequence
	frequenceHz = frequence[0] * 10000000 + frequence[1] * 1000000 + frequence[3] * 100000 + frequence[4] * 10000 + frequence[5] * 1000 + frequence[7] * 100 + frequence[8] * 10 + frequence[9];
	gen.ApplySignal(typeSignal, REG0, frequenceHz);
	
	// Debug ON
	Serial.print("    Valeur frequenceHz: ");
	Serial.println(frequenceHz);
	Serial.println("   <-- setFrequence");
	// Debug OFF
}

// ********************************
// Programmation de l'amplificateur
// ********************************
void setAmplificateur(void) {
	// Debug ON
	Serial.println("   --> setAmplificateur");
	// Debug OFF

	if(signal) {
		// Debug ON
		Serial.print("    Signal: ");
		Serial.println(typeSignal);
		// Debug OFF
		
		switch(typeSignal) {
			case SINE_WAVE    :
			case TRIANGLE_WAVE: aop.setCanal(canalSinusTriangle[indiceTension]);
								aop.setGain(gainSinusTriangle[indiceTension]);
								
								// Debug ON								
								Serial.print("    Valeur canal: ");
								Serial.println(canalSinusTriangle[indiceTension]);
								Serial.print("    Valeur gain: ");
								Serial.println(gainSinusTriangle[indiceTension]);
								// Debug OFF

								break;
			case SQUARE_WAVE  : aop.setCanal(canalCarre[indiceTension]);
								aop.setGain(gainCarre[indiceTension]);
								
								// Debug ON								
								Serial.print("    Valeur canal: ");
								Serial.println(canalCarre[indiceTension]);
								Serial.print("    Valeur gain: ");
								Serial.println(gainCarre[indiceTension]);
								// Debug OFF

								break;
		}

	} else {
		// Debug ON
		Serial.println("    Signal: OFF");
		// Debug OFF

		// Tension minimale
		aop.setCanal(aop.In_4);
		aop.setGain(aop.Gain_1);
	}

	// Debug ON
	Serial.println("   <-- setAmplificateur");
	// Debug OFF
}

