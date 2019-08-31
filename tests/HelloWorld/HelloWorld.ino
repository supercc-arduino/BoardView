#include <ESP8266WiFi.h>

#define SERIAL_SPEED 115200

/*
 * 
 * 
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
}

void loop() {			
	Serial.println("Hello world");
	delay(1000);
}
