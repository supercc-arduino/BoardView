#include <BoardView.h>

#define SERIAL_SPEED 115200
#define MAX_LINE_LEN 200

// Connexion wifi

/*
 * Définissez WIFI_SSID et WIFI_PASSWORD dans le fichier private.h ou 
 * définissez les dans ce fichier et commentez l'inclusion.
 * 
 * Set WIFI_SSID and WIFI_PASSWORD in the prefs.h file or set them in 
 * this file and comment on the inclusion.
 * 
 * #define WIFI_SSID "YOUR-WIFI-SSID"
 * #define WIFI_PASSWORD "YOUR-WIFI-PASSWORD"
 * 
 * */

#include "private.h"

// Objet boardView, utilisé pour les communications.

BoardView boardView; 

// autres variables gloables

float hum;               // variable reflétant l'état du capteur d'humidité.
float threshold=80;      // seuil au dessus duquel l'extraction est souhaité.
float margin=2;          // marge autour du seul pour éviter les 
                         // oscillations trop rapides
                         
int relay;               // variable reflétant l'état du relais (extracteur).    
int onOff;               // variable indiquant si la régulation doit être 
                         // activée : activée si 1, désactivée si 0.

void update() {
	// cf  https://www.arduino.cc/en/tutorial/BlinkWithoutDelay
	
	// each 10 ms
	static unsigned long interval = 10, previousMillis=0; 
	
	
	unsigned long currentMillis = millis();
	if (currentMillis - previousMillis >= interval) {
		previousMillis = currentMillis;
	
		// update
		// ici je simule (toutes les 10 ms) le capteur avec l'évolution 
		// de l'humidité suivante (taux d'humidité : %)
		// Si le relay n'est pas activé, l'humidité grimpe de 0.2 %
		// sinon l'humidité chute de 0.1 %
		// Si elle dépasse 100°C elle est ramenée à 0 °C
		// si elle passe sous 0 °C elle passe repasse à 100 °C
		
		if(!relay) hum=hum+0.2;
			else hum=hum-0.1;
		if(hum>100) hum=0;
		if(hum<0) hum=100;		
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
		addFloat(response, MAX_LINE_LEN, "hum", hum, 1);
		addFloat(response, MAX_LINE_LEN, "threshold", threshold, 1);
		addFloat(response, MAX_LINE_LEN, "margin", margin, 1);
		addInt  (response, MAX_LINE_LEN, "relay", relay);
		addInt  (response, MAX_LINE_LEN, "onOff", onOff);
		
	}
	
	// Accesseurs en écriture
	// On traite tous les messages de la forme varName=value
	// Les fonctions (de BoardViewProto.cpp)  de la forme matchAndAssign*
	// font cela très bien.
	
	else if(matchAndAssignFloat(request, "hum", &hum)) strcpy(response, "ok"); 
	else if(matchAndAssignFloat(request, "threshold", &threshold)) strcpy(response, "ok"); 
	else if(matchAndAssignFloat(request, "margin", &margin)) strcpy(response, "ok"); 
	else if(matchAndAssignInt  (request, "relay", &relay)) strcpy(response, "ok");
	else if(matchAndAssignInt  (request, "onOff", &onOff)) strcpy(response, "ok");
		
	// Autres commandes
	
	// ...
	
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
	boardView.parseRequest=parseRequest; // mini interpréteur de commandes
	boardView.maxLineLen=MAX_LINE_LEN;
	boardView.name="Bang-bang";
	boardView.fontSize=2.0;
	boardView.viewRefreshPeriodMs=100;

	boardView.addLabel("hum");
	boardView.addEntry("threshold");
	boardView.addEntry("margin");
	
    boardView.addCheckBox("relay", "relay=0", "relay=1");
	boardView.addCheckBox("onOff", "onOff=0; relay=0;", "onOff=1");

	boardView.begin(); 
	
}




void loop() {			

	// lecture des capteurs (mise à jour des variables
	// qui reflètent leur état).
	
	update();
	
	// traitement des messages en attente en provenance de la web socket.
	
	boardView.loop();

	
	// régulation
	
	if(hum < threshold-margin) {
		if(onOff) relay=0;
	}
	else if(hum > threshold+margin) {
		if(onOff) relay=1;
	}
	
	// mise à jour des actionneurs (logique négative pour led d1-min)
	
	digitalWrite(LED_BUILTIN, !relay); 

}

