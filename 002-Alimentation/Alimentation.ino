// ****************
// Alimentation
// totof 2018/12/08
// ****************

// Liste des fonctions
constexpr auto CONTRASTE = 0;
constexpr auto LUMINOSITE = 1;
constexpr auto CONT_LUM_AUTO = 2;
constexpr auto SORTIE1 = 3;
constexpr auto SORTIE2 = 4;
constexpr auto VERROUILLAGE_SORTIES = 5;

// List des sorties
constexpr auto OFF = 0;
constexpr auto TENSION = 1;
constexpr auto COURANT = 2;
constexpr auto PUISSANCE = 3;

// List des pin
constexpr auto RELAIS_SORTIE1 = 2;
constexpr auto RELAIS_SORTIE2 = 9;
constexpr auto TOUCHE_FONCTION = 16;
constexpr auto TOUCHE_MOINS = 15;
constexpr auto TOUCHE_PLUS = 14;

// Liste des potentiometre
constexpr auto POTENTIOMETRE_CONTRASTE = 1;
constexpr auto POTENTIOMETRE_LUMIERE = 2;

// Librairies
#include <Wire.h>
#include <LiquidCrystal.h>
#include "Adafruit_INA219.h"
#include "MCP42010.h"

// Peripheriques
Adafruit_INA219 out1 = Adafruit_INA219(0x44);
Adafruit_INA219 out2 = Adafruit_INA219(0x45);
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);

// Fonction, initialise à SORTIE 1 pour l'affichage
uint8_t fonction = SORTIE1;

// Luminosite initialise au max
uint8_t valLumiere = 0;

// Contraste initialise au max
uint8_t valContraste = 0;

// Lumiere contraste auto
bool lumContrasteAuto = true;

// Sortie 1
uint8_t valSortie1 = OFF;

// Sortie2
uint8_t valSortie2 = OFF;

// Verrouillage sorties
bool verrouillageSorties = false;

// Memorise l'etat des touches pour detecte le changement d'etat
int valPrevTouchSwitchFnct = LOW;
int valPrevTouchSwitchFnctMoins = LOW;
int valPrevTouchSwitchFnctPlus = LOW;


// *****************************************
//       ***** ***** ***** *   * *****
//       *     *       *   *   * *   *
//       ***** ****    *   *   * *****
//           * *       *   *   * *
//       ***** *****   *   ***** *
// *****************************************
void setup() {
	Serial.begin(115200);

	// Initialisation afficheur avec luminosite et contraste a fond
	MCP42010 pot = MCP42010(10, 13, 11);
	pot.setPot(POTENTIOMETRE_CONTRASTE, valContraste);
	pot.setPot(POTENTIOMETRE_LUMIERE, valLumiere);
	lcd.begin(16, 2);
	// Message d'initialisation
	lcd.setCursor(0, 0);
    lcd.print("  Alimentation  ");
	lcd.setCursor(0, 1);
	lcd.print("V1.00   Init. OK");
	Serial.println("Init afficheur");
	lcd.noCursor();

	// Initialise les mesureurs
	Serial.println("Init mesureurs");
	out1.begin();
	out2.begin();

	// Initialise les pin
	pinMode(RELAIS_SORTIE1, OUTPUT);
	pinMode(RELAIS_SORTIE2, OUTPUT);
	pinMode(TOUCHE_FONCTION, INPUT);
	pinMode(TOUCHE_MOINS, INPUT);
	pinMode(TOUCHE_PLUS, INPUT);

 	// Delay d'init
	delay(4000);
	Serial.println("Fin init");
}

// ****************************************
//         *     ***** ***** *****
//         *     *   * *   * *   *
//         *     *   * *   * *****
//         *     *   * *   * *
//         ***** ***** ***** *
// ****************************************

void loop() {
	// *******************
	// Lecture des touches
	// *******************

	// Mesure
	float mesure;

	// Action sur la touche fonction
	if (touchSwitchFnct()) {
		fonction = rotateFonction(fonction);
	}

	// Action sur la touche moins
	if (touchSwitchFnctMoins()) {
		switch (fonction) {
		case CONTRASTE:
			valContraste = downContraste();
			break;
		case LUMINOSITE:
			valLumiere = downLumiere();
			break;
		case CONT_LUM_AUTO:
			lumContrasteAuto = changeContrasteAuto();
			break;
		case SORTIE1:
			valSortie1 = rotateDownSortie1();
			break;
		case SORTIE2:
			valSortie2 = rotateDownSortie2();
			break;
		case VERROUILLAGE_SORTIES:
			verrouillageSorties = changeVerrouillageSorties();
			break;
		}
	}

	// Action sur la touche plus
	if (touchSwitchFnctPlus()) {
		switch (fonction) {
		case CONTRASTE:
			valContraste = upContraste();
			break;
		case LUMINOSITE:
			valLumiere = upLumiere();
			break;
		case CONT_LUM_AUTO:
			lumContrasteAuto = changeContrasteAuto();
			break;
		case SORTIE1:
			valSortie1 = rotateUpSortie1();
			break;
		case SORTIE2:
			valSortie2 = rotateUpSortie2();
			break;
		case VERROUILLAGE_SORTIES:
			verrouillageSorties = changeVerrouillageSorties();
			break;
		}
	}

	// ************************
	// Action sur les appareils
	// ************************
	// Luminosite contraste afficheur
	if (lumContrasteAuto) {
		// Calcul de la lumiere et du contraste si auto
		valContraste = analogRead(3) / 10;;
		valLumiere = valContraste;
		Serial.println("Auto");
	}
	MCP42010 pot = MCP42010(10, 13, 11);
	pot.setPot(POTENTIOMETRE_CONTRASTE, valContraste);
	Serial.print("Contraste = ");
	Serial.println(valContraste);
	pot.setPot(POTENTIOMETRE_LUMIERE, valLumiere);
	Serial.print("Lumiere = ");
	Serial.println(valLumiere);

	// Mesureur
	switch (fonction) {
	case SORTIE1:
		switch (valSortie1) {
		case OFF: 
			sortie1Off();
			// Delay pour les relais
			delay(10);
			break;
		case TENSION: 
			sortie1On();
			// Delay pour les relais
			delay(10);
			mesure = out1.getBusVoltage_V();
			break;
		case COURANT: 
			sortie1On();
			// Delay pour les relais
			delay(10);
			mesure = out1.getCurrent_mA();
			break;
		case PUISSANCE:  
			sortie1On();
			// Delay pour les relais
			delay(10);
			mesure = out1.getPower_mW();
			break;
		}
		break;
	case SORTIE2:
		switch (valSortie2) {
		case OFF:
			sortie2Off();
			// Delay pour les relais
			delay(10);
			break;
		case TENSION:
			sortie2On();
			// Delay pour les relais
			delay(10);
			mesure = out2.getBusVoltage_V();
			break;
		case COURANT:
			sortie2On();
			// Delay pour les relais
			delay(10);
			mesure = out2.getCurrent_mA();
			break;
		case PUISSANCE:
			sortie2On();
			// Delay pour les relais
			delay(10);
			mesure = out2.getPower_mW();
			break;
		}
		break;
	}
	
	// *********
	// Affichage
	// *********
	switch (fonction) {
	case CONTRASTE:
		afficheContraste(valContraste);
		break;
	case LUMINOSITE:
		afficheLumiere(valLumiere);
		break;
	case CONT_LUM_AUTO:
		afficheContLumAuto(lumContrasteAuto);
		break;
	case SORTIE1: 
		afficheSortie(fonction, valSortie1, mesure);
		break;
	case SORTIE2: 
		afficheSortie(fonction, valSortie2, mesure);
		break;
	case VERROUILLAGE_SORTIES: 
		afficheVerrouillageSortie(verrouillageSorties);
		break;
	}

	// *************
	// Temporisation
	// *************
	delay(300);
}

// ********************************************************************
//         ***** ***** *   * ***** *****   *  ***** *   * *****
//         *     *   * **  * *       *     *  *   * **  * *
//         ***   *   * * * * *       *     *  *   * * * * *****
//         *     *   * *  ** *       *     *  *   * *  **     *
//         *     ***** *   * *****   *     *  ***** *   * *****
// ********************************************************************

// ******************
// Affiche sortie
// @Param la fonction
// @Param la sortie
// @Param la valeur
// ******************
void afficheSortie(uint8_t fonct, uint8_t sortie, float valeur) {
	Serial.println("--> afficheSortie");
	lcd.setCursor(0, 0);
	switch (fonct) {
	case SORTIE1:
		lcd.print("Sortie 1        ");
		break;
	case SORTIE2:
		lcd.print("Sortie 2        ");
		break;
	}
	lcd.setCursor(0, 1);
	switch (sortie) {
	case OFF:
		lcd.print("Off             ");
		break;
	case TENSION:
		lcd.print(valeur);
		lcd.setCursor(4, 1);
		lcd.print(" V          ");
		break;
	case COURANT:
		lcd.print(valeur);
		// Mesure superieure a 1000
		lcd.setCursor(7, 1);
		lcd.print(" mA       ");
		// Entre 1000 et 100
		if (valeur < 1000) {
			lcd.setCursor(6, 1);
			lcd.print(" mA        ");
		}
		// Entre 100 et 10
		if (valeur < 100) {
			lcd.setCursor(5, 1);
			lcd.print(" mA         ");
		}
		// Inferieure a 10
		if (valeur < 10) {
			lcd.setCursor(4, 1);
			lcd.print(" mA          ");
		}
		break;
	case PUISSANCE:
		lcd.print(valeur);
		// Mesure superieure a 1000
		lcd.setCursor(7, 1);
		lcd.print(" mW       ");
		// Entre 1000 et 100
		if (valeur < 1000) {
			lcd.setCursor(6, 1);
			lcd.print(" mW        ");
		}
		// Entre 100 et 10
		if (valeur < 100) {
			lcd.setCursor(5, 1);
			lcd.print(" mW         ");
		}
		// Inferieure a 10
		if (valeur < 10) {
			lcd.setCursor(4, 1);
			lcd.print(" mW          ");
		}
		break;
	}
	Serial.println("<-- afficheSortie");
}

// ***********************************
// Affichage du contraste lumiere auto
// @Param la valeur auto
// ***********************************
void afficheContLumAuto(bool valAuto) {
	Serial.println("--> afficheContLumAuto");
	lcd.setCursor(0, 0);
	lcd.print("Cont/Lum Auto   ");
	lcd.setCursor(0, 1);
	if (valAuto) {
		lcd.print("On              ");
	}
	else {
		lcd.print("Off             ");
	}
	Serial.println("<-- afficheContLumAuto");
}

// **************************************
// Affichage du verrouillage de la sortie
// @Param la valeur de verrouillage
// **************************************
void afficheVerrouillageSortie(bool valVerrouillage) {
	Serial.println("--> afficheVerrouillageSortie");
	lcd.setCursor(0, 0);
	lcd.print("Verrouillage    ");
	lcd.setCursor(0, 1);
	if (valVerrouillage) {
		lcd.print("On              ");
	}
	else {
		lcd.print("Off             ");
	}
	Serial.println("<-- afficheVerrouillageSortie");
}

// ***********************
// Affichage de la lumiere
// @Param la lumiere
// ***********************
void afficheLumiere(uint8_t lumiere) {
	Serial.println("--> afficheLumiere");
	lcd.setCursor(0, 0);
	lcd.print("Luminosite      ");
	char charVal[4];
	sprintf(charVal, "%d", lumiere);
	lcd.setCursor(0, 1);
	lcd.print("Valeur : ");
	lcd.setCursor(9, 1);
	lcd.print(charVal);
	// Un ou deux chiffre
	if(lumiere < 10) {
		lcd.setCursor(10, 1);
	} else {
		lcd.setCursor(11, 1);		
	}
	lcd.print("      ");
	Serial.println("<-- afficheLumiere");
}

// **********************
// Affichage du contraste
// @Param le contraste
// **********************
void afficheContraste(uint8_t contraste) {
	Serial.println("--> afficheContraste");
	lcd.setCursor(0, 0);
	lcd.print("Contraste       ");
	char charVal[4];
	sprintf(charVal, "%d", contraste);
	lcd.setCursor(0, 1);
	lcd.print("Valeur : ");
	lcd.setCursor(9, 1);
	lcd.print(charVal);
	// Un ou deux chiffre
	if(contraste < 10) {
		lcd.setCursor(10, 1);
	} else {
		lcd.setCursor(11, 1);		
	}
	lcd.print("      ");
	Serial.println("<-- afficheContraste");
}

// ****************
// Sortie 1 sur Off
// ****************
void sortie1Off(void) {
	Serial.println("--> sortie1Off");
	digitalWrite(RELAIS_SORTIE1, LOW);
	Serial.println("<-- sortie1Off");
}

// ***************
// Sortie 1 sur On
// ***************
void sortie1On(void) {
	Serial.println("--> sortie1On");
	digitalWrite(RELAIS_SORTIE1, HIGH);
	Serial.println("<-- sortie1On");
}

// ****************
// Sortie 2 sur Off
// ****************
void sortie2Off(void) {
	Serial.println("--> sortie2Off");
	digitalWrite(RELAIS_SORTIE2, LOW);
	Serial.println("<-- sortie2Off");
}

// ***************
// Sortie 2 sur On
// ***************
void sortie2On(void) {
	Serial.println("--> sortie2On");
	digitalWrite(RELAIS_SORTIE2, HIGH);
	Serial.println("<-- sortie2On");
}

// ****************************
// Rotation down de la sortie 1
// @Return La nouvelle sortie 1
// ****************************
uint8_t rotateDownSortie1(void) {
	Serial.println("--> rotateDownSortie1");
	Serial.println("<-- rotateDownSortie1");
	return rotateDownSortie(valSortie1);
}

// ****************************
// Rotation down de la sortie 2
// @Return La nouvelle sortie 2
// ****************************
uint8_t rotateDownSortie2(void) {
	Serial.println("--> rotateDownSortie2");
	Serial.println("<-- rotateDownSortie2");
	return rotateDownSortie(valSortie2);
}

// **************************
// Rotation down d'une sortie
// @Return La nouvelle sortie
// **************************
uint8_t rotateDownSortie(uint8_t valeur) {
	Serial.println("--> rotateDownSortie");
	switch (valeur) {
	case OFF: 
		Serial.println("<-- rotateDownSortie"); 
		return TENSION;
	case TENSION: 
		Serial.println("<-- rotateDownSortie"); 
		return COURANT;
	case COURANT: 
		Serial.println("<-- rotateDownSortie"); 
		return PUISSANCE;
	case PUISSANCE:  
		Serial.println("<-- rotateDownSortie"); 
		// Si les sorties sont verouille, on evite le OFF
		if(verrouillageSorties) {
			return TENSION;
		} else {
			return OFF;
		}
	}
}

// ****************************
// Rotation up de la sortie 1
// @Return La nouvelle sortie 1
// ****************************
uint8_t rotateUpSortie1(void) {
	Serial.println("--> rotateUpSortie1");
	Serial.println("<-- rotateUpSortie1");
	return rotateUpSortie(valSortie1);
}

// ****************************
// Rotation up de la sortie 2
// @Return La nouvelle sortie 2
// ****************************
uint8_t rotateUpSortie2(void) {
	Serial.println("--> rotateUpSortie2");
	Serial.println("<-- rotateUpSortie2");
	return rotateUpSortie(valSortie2);
}

// **************************
// Rotation up d'une sortie
// @Return La nouvelle sortie
// **************************
uint8_t rotateUpSortie(uint8_t valeur) {
	Serial.println("--> rotateUpSortie");
	switch (valeur) {
	case OFF: 
		Serial.println("<-- rotateUpSortie"); 
		return PUISSANCE;
	case TENSION: 
		Serial.println("<-- rotateUpSortie"); 
		// Si les sorties sont verouille, on evite le OFF
		if(verrouillageSorties) {
			return PUISSANCE;
		} else {
			return OFF;
		}
	case COURANT: 
		Serial.println("<-- rotateUpSortie"); 
		return TENSION;
	case PUISSANCE: 
		Serial.println("<-- rotateUpSortie"); 
		return COURANT;
	}
}

// *********************************
// Inverse le contraste auto
// @Return le contraste auto inverse
// *********************************
bool changeContrasteAuto(void) {
	Serial.println("--> changeContrasteAuto");
	Serial.println("<-- changeContrasteAuto");
	return !lumContrasteAuto;
}

// ******************************************
// Inverse le verrouillage des sorties
// @Return le verrouillage des sortie inverse
// ******************************************
bool changeVerrouillageSorties(void) {
	Serial.println("--> changeVerrouillageSorties");
	Serial.println("<-- changeVerrouillageSorties");
	return !verrouillageSorties;
}

// ****************************************
// Rotation de la touche fonction
// @Return la nouvelles valeur de la touche
// ****************************************
uint8_t rotateFonction(uint8_t valeur) {
	Serial.println("--> rotateFonction");
	valeur++;
	if (valeur > VERROUILLAGE_SORTIES) {
		valeur = CONTRASTE;
	}
	Serial.println("<-- rotateFonction");
	return valeur;
}

// *************************************************
// Baisse le contraste
// @Return la valeur du contraste baisse si possible
// *************************************************
uint8_t downContraste(void) {
	Serial.println("--> downContraste");
	lumContrasteAuto = false;
	Serial.println("<-- downContraste");
	return downAfficheur(valContraste);
}

// **************************************************
// Baisse la lumiere
// @Return la valeur de la lumiere baisse si possible
// **************************************************
uint8_t downLumiere(void) {
	Serial.println("--> downLumiere");
	lumContrasteAuto = false;
	Serial.println("<-- downLumiere");
	return downAfficheur(valLumiere);
}

// ************************************
// Baisse une valeur de l'afficheur
// @Return la valeur baisse si possible
// ************************************
uint8_t downAfficheur(uint8_t valeur) {
	Serial.println("--> downAfficheur");
	// 0 est le contraste et la luminosite max
	if (valeur > 5) {
		valeur -= 5;
	} else {
		valeur = 0;
	}
	Serial.print("Valeur = ");
	Serial.println(valeur);
	Serial.println("<-- downAfficheur");
	return valeur;
}

// ***************************************************
// Augmente le contraste
// @Return la valeur du contraste augmente si possible
// ***************************************************
uint8_t upContraste(void) {
	Serial.println("--> upContraste");
	lumContrasteAuto = false;
	Serial.println("<-- upContraste");
	return upAfficheur(valContraste);
}

// ****************************************************
// Augmente la lumiere
// @Return la valeur de la lumiere augmente si possible
// ****************************************************
uint8_t upLumiere(void) {
	Serial.println("--> upLumiere");
	lumContrasteAuto = false;
	Serial.println("<-- upLumiere");
	return upAfficheur(valLumiere);
}

// **************************************
// Augmente une valeur de l'afficheur
// @Return la valeur augmente si possible
// **************************************
uint8_t upAfficheur(uint8_t valeur) {
	Serial.println("--> upAfficheur");
	// 80 est le contraste et la luminosite min (quasi eteint)
	if (valeur < 80) {
		valeur += 5;
	}
	Serial.print("Valeur = ");
	Serial.println(valeur);
	Serial.println("<-- upAfficheur");
	return valeur;
}

// *******************************************************************
// Valeur de la touche des fonctions
// @Return l'etat de la touche avec historique
// @Remark table de verite
// valeur precedente    valeur lue Resultat Explication
// LOW                  LOW        false    Pas d'action
// HIGH                 LOW        false    Touche relache
// LOW                  HIGH       true     Touche vient d'etre active
// HIGH                 HIGH       false    Action deja traitee
// *******************************************************************
bool touchSwitchFnct(void) {
	Serial.println("--> touchSwitchFnct");
	bool valReturn = false;
	int valeur = digitalRead(TOUCHE_FONCTION);
	if (valeur == HIGH && valPrevTouchSwitchFnct == LOW) {
		valReturn = true;
	}
	valPrevTouchSwitchFnct = valeur;
	Serial.println("<-- touchSwitchFnct");
	return valReturn;
}

// *******************************************************************
// Valeur de la touche des fonctions moins
// @Return l'etat de la touche avec historique
// @Remark table de verite
// valeur precedente    valeur lue Resultat Explication
// LOW                  LOW        false    Pas d'action
// HIGH                 LOW        false    Touche relache
// LOW                  HIGH       true     Touche vient d'etre active
// HIGH                 HIGH       false    Action deja traitee
// *******************************************************************
bool touchSwitchFnctMoins(void) {
	Serial.println("--> touchSwitchFnctMoins");
	bool valReturn = false;
	int valeur = digitalRead(TOUCHE_MOINS);
	if (valeur == HIGH && valPrevTouchSwitchFnctMoins == LOW) {
		valReturn = true;
	}
	valPrevTouchSwitchFnctMoins = valeur;
	Serial.println("<-- touchSwitchFnctMoins");
	return valReturn;
}

// *******************************************************************
// Valeur de la touche des fonctions plus
// @Return l'etat de la touche avec historique
// @Remark table de verite
// valeur precedente    valeur lue Resultat Explication
// LOW                  LOW        false    Pas d'action
// HIGH                 LOW        false    Touche relache
// LOW                  HIGH       true     Touche vient d'etre active
// HIGH                 HIGH       false    Action deja traitee
// *******************************************************************
bool touchSwitchFnctPlus(void) {
	Serial.println("--> touchSwitchFnctPlus");
	bool valReturn = false;
	int valeur = digitalRead(TOUCHE_PLUS);
	if (valeur == HIGH && valPrevTouchSwitchFnctPlus == LOW) {
		valReturn = true;
	}
	valPrevTouchSwitchFnctPlus = valeur;
	Serial.println("<-- touchSwitchFnctPlus");
	return valReturn;
}

