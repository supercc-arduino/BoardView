#include <BoardView.h>

#define SERIAL_SPEED 115200
#define MAX_LINE_LEN 200

/*
 * 
 Pont websocket vers série (UART) pour ChronoNano.
 * */

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
	
	// Mode "redirection" : le programme ne traite plus lui même les
	// messages arrivant via le réseau, 
	// il se contente de les rediriger vers le flux (Stream) indiqué,
	// de capturer les réponses en provenance du flux pour les transmettre
	// au demandeur initial.
	
	boardView.enabledRedirect(Serial);
	boardView.maxLineLen=MAX_LINE_LEN;
	boardView.name="Chrono-nano";	
	boardView.fontSize=2.0;
	boardView.viewRefreshPeriodMs=100;

	boardView.addLabel("chrono");
	boardView.addCheckBox("startStop", "startStop=0", "startStop=1");
	boardView.addButton("Clear", "startStop=0; clear");	
	
	// démarrage des services de boardView
	
	boardView.begin(); 	
}


void loop() {			

	// traitement des messages en attente en provenance de la web socket.
	
	boardView.loop();
}

