#include <BoardView.h>

#define SERIAL_SPEED 115200
#define MAX_LINE_LEN 200

/*
 * 
 * On souhaite controler depuis son navigateur Internet un chronomètre situé 
 * sur un esp8266. On souhaite disposer d'un bouton Start/Stop et d'un bouton 
 * Clear. Le bouton clear, outre le fait de remettre à 0 le chronomètre 
 * l'arrête également.
 * 
 * */

// Connexion wifi

/*
 * Définissez WIFI_SSID et WIFI_PASSWORD dans le fichier BoardViewPrivate.h ou 
 * définissez les dans ce fichier et commentez l'inclusion.
 * 
 * Set WIFI_SSID and WIFI_PASSWORD in the prefs.h file or set them in 
 * this file and comment on the inclusion.
 * 
 * #define WIFI_SSID "YOUR-WIFI-SSID"
 * #define WIFI_PASSWORD "YOUR-WIFI-PASSWORD"
 * 
 * */

#include "BoardViewPrivate.h"

// Variables globales (initialisées par défaut avec des 0)

float chrono;            // la valeur du chronomètre.
int startStop;           // l'état démarré/arrêté (géré par l'utilisateur).

// Objet boardView, utilisé pour les communications.
BoardView boardView; 

// Fonction de mise à jour périodique.
// Mise à jour de la valeur du compteur ici

void updateTime() {

	// each 10 ms : voir  https://www.arduino.cc/en/tutorial/BlinkWithoutDelay

	static unsigned long interval = 10, previousMillis=0; 

	unsigned long currentMillis = millis();
	if (currentMillis - previousMillis >= interval) {
		previousMillis = currentMillis;

		digitalWrite(LED_BUILTIN, !startStop); // logique negative sur d1-mini.
		if(startStop==1) chrono+=0.01;

	}
}

// mini interpréteur de commandes simples de la forme :
// command arg1 arg2 ...
// variable=valeur

int parseRequest(char *request, char *response, unsigned len) {
	int ret=0; // code de retour 0 : ok, sinon code d'erreur.

	response[0]=0; //clear response

	// lowercase first letter (for smartphone keyboard)
	if(request[0]>='A' && request[0]<='Z') request[0]=request[0]+'a'-'A';

	// On utilise les fonctions utilitaires de codage/décodage d'une commande

	// commande dump
	// La commande dump doit retourner dans la variable response la liste des
	// couple varName=value que la carte souhaite exposer en lecture seule à 
	// l'extérieur.

	if(matchCmd(request,"dump")) {
		// On ajoute toutes les variables que l'on souhaite rendre visibles
		// L'ordre n'a pas d'importance.
		addFloat(response, MAX_LINE_LEN, "chrono", chrono, 3);
		addInt(response, MAX_LINE_LEN, "startStop", startStop);
	}

	// Accesseurs en écriture
	// On traite tous les messages de la forme varName=value
	// Les fonctions (de BoardViewProto.cpp)  de la forme matchAndAssign*
	// font cela très bien.

	else 
		if(matchAndAssignInt(request, "startStop", &startStop)) 
			strcpy(response, "ok");

	// Autres commandes

	else if(matchCmd(request,"clear")) { startStop=0, chrono=0; response="ok"; }

	// Si la commande n'est pas reconnue, message d'erreur

	else { ret=1; sprintf(response, "Error : bad request : %s", request); }

	return ret; 
}

void setup() {

	Serial.begin(SERIAL_SPEED);

	pinMode(LED_BUILTIN, OUTPUT);

	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) delay(500); 

	Serial.println("Wifi config :");
	Serial.println("-----------");
	Serial.print("Wifi : AP  name : "); Serial.println(WIFI_SSID);
	Serial.print("Wifi : IP  addr : "); Serial.println(WiFi.localIP());
	Serial.print("Wifi : MAC addr : "); Serial.println(WiFi.macAddress());
	Serial.println("");	


	// Configuration de boardView

	// Notre mini interpréteur de commandes
	boardView.parseRequest=parseRequest;
	boardView.maxLineLen=MAX_LINE_LEN;

	boardView.name="Chrono-1";	
	boardView.fontSize=2.0;
	boardView.viewRefreshPeriodMs=100;

	boardView.addLabel("chrono");
	boardView.addCheckBox("startStop", "startStop=0", "startStop=1");
	boardView.addButton("Clear", "clear");	

	// démarrage des services de boardView

	boardView.begin(); 	
}

void loop() {			

	// lecture des capteurs (mise à jour des variables
	// qui reflètent leur état).

	updateTime();

	// traitement des messages en attente en provenance de la web socket.

	boardView.loop();

}
