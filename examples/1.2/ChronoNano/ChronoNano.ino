#include <BoardViewProto.h>

#define SERIAL_SPEED 115200
#define MAX_LINE_LEN 200

/*
 * 
 * On souhaite controler depuis son navigateur Internet un chronomètre situé 
 * sur une nano. On souhaite disposer d'un bouton Start/Stop et d'un bouton 
 * Clear. Le bouton clear, outre le fait de remettre à 0 le chronomètre 
 * l'arrête également.
 * 
 * La connexion de la nano au wifi est réalisée par le mode "redirect" d'un 
 * objet boardView sur un esp8266.
 * 
 * */

// Variables globales (initialisées par défaut avec des 0)

float chrono;            // la valeur du chronomètre.
int startStop;           // l'état démarré/arrêté (géré par l'utilisateur).

// Fonction de mise à jour périodique.
// Mise à jour de la valeur du compteur ici

void updateTime() {

	// each 10 ms : voir  https://www.arduino.cc/en/tutorial/BlinkWithoutDelay

	static unsigned long interval = 10, previousMillis=0; 

	unsigned long currentMillis = millis();
	if (currentMillis - previousMillis >= interval) {
		previousMillis = currentMillis;

		digitalWrite(LED_BUILTIN, startStop);
		if(startStop==1) chrono+=0.01;

	}
}

// mini interpréteur de commandes simples de la forme :
// command arg1 arg2 ...
// variable=valeur

int parseRequest(char *request, char *response, int len) {
	int ret=0; // code de retour 0 : ok, sinon code d'erreur.

	response[0]=0; //clear response

	// lowercase first letter (for smartphone keyboard)
	if(request[0]>='A' && request[0]<='Z') request[0]=request[0]+'a'-'A';

	// On utilise les fonctions utilitaires de codage/décodage d'une commande

	// commande dump
	// La commande dump doit retourner dans la variable response la liste des
	// couple varName=value que la carte souhaite exposer en lecture seule à 
	// l'extérieur.

	if(matchCmd(request,(char *)"dump")) {
		// On ajoute toutes les variables que l'on souhaite rendre visibles
		// L'ordre n'a pas d'importance.
		addFloat(response, MAX_LINE_LEN, (char *)"chrono", chrono, 3);
		addInt(response, MAX_LINE_LEN, (char *)"startStop", startStop);
	}

	// Accesseurs en écriture
	// On traite tous les messages de la forme varName=value
	// Les fonctions (de BoardViewProto.cpp)  de la forme matchAndAssign*
	// font cela très bien.

	else 
		if(matchAndAssignInt(request, (char *)"startStop", &startStop)) 
			strcpy(response,(char *)"ok");

	// Autres commandes

	else if(matchCmd(request,(char *)"clear")) { startStop=0, chrono=0; response=(char *)"ok"; }

	// Si la commande n'est pas reconnue, message d'erreur

	else { ret=1; sprintf(response, "Error : bad request : %s", request); }

	return ret; 
}

void setup() {

	Serial.begin(SERIAL_SPEED);

	pinMode(LED_BUILTIN, OUTPUT);	
}

void loop() {			

	// lecture des capteurs (mise à jour des variables
	// qui reflètent leur état).

	updateTime();

	// traitement des messages en attente en provenance de la 
	// liaison série.

	readAndParseLines(Serial, parseRequest, MAX_LINE_LEN);
}
