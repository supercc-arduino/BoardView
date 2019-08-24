# BoardView

WARNING, la doc est en avance par rapport au code. Le code de TCP n'est pas encore posté donc les explications sur TCP ne s'appliquent pas encore. Je fais la doc d'abord...

- [BoardView](#boardview)
  - [Introduction](#introduction)
  - [Installation](#installation)
  - [La vue](#la-vue)
  - [Le langage de commandes](#le-langage-de-commandes)
  - [Exemples d'interactions via le navigateur](#exemples-d&apos;interactions-via-le-navigateur)
    - [Exemple 1.1 : Un chronomètre.](#exemple-11--un-chronomètre)
    - [Exemple 1.2 : Le chronomètre déporté sur une nano](#exemple-12--le-chronomètre-déporté-sur-une-nano)
    - [Exemple 1.3 : Un asservissement tout ou rien](#exemple-13--un-asservissement-tout-ou-rien)
  - [FAQ](#FAQ)

## Introduction

L'idée de BoardView est d'offrir des fonctions/classes "Réseaux" facilement accessibles à ceux qui veulent faire de la communication réseau avec leurs arduinos sans vouloir maîtriser complètement les aspects techniques des protocoles réseaux de la famille TCP/IP utilisés (HTTP, WebSocket, TCP). 

Exemple d'applications :
  * Un contrôle interactif de ses cartes avec son smartphone (connecté au point d'accès partagé par les noeuds).
  * Une solution domotique avec ses propre serveurs.
  * De la robotique (commande du robot via son PC, une Raspberry, ...)
  * Des automates "dispersés" (des capteurs sont sur des cartes et des actionneurs sur d'autres par exemple).
  
Quelques caractéristiques :

  * n'a été testé que sur ESP8266. Projet très jeune, mais les exemples fonctionnent.
  
  * utilise un réseau wifi entre les participants supposé existant et acceptant l'intégration des noeuds (cartes). Une box domestique fait l'affaire. 
  
  * dispose d'un logiciel pont, permettant à n'importe qu'elle carte arduino connecté simplement via un port série (UART) à l'ESP8266 de réaliser toutes les opérations.

  * ne bride en rien les capacité du réseau existant (vos cartes pourront toujours avoir accès à Internet ou à tout autre service si c'était le cas avant).
  
  * Utilise un mini interpréteur de commandes facilement personnalisable.
  
  * les cartes peuvent être commandées simulanément via les différents protocoles supportés. Cependant les différentes requêtes arrivant simultanément sur un même noeud seront éxécutées séquentiellement (dans un ordre indéterminé) afin d'assurer leur atomicité.
  
Voir aussi les [##FAQ]

Au niveau du code, BoardView est une classe C++ pour ESP8266 permettant de communiquer avec vos cartes arduinos à l'aide d'un navigateur Web (de façon interactive via l'usage d'une web socket) et/ou  via une connexion TCP (pour l'automatisation du pilotage/contrôle des cartes par d'autre cartes et/ou avec d'autres programmes (scripts, Ruby, ...) sur PC ou Raspberry PI (et de façon générale sous systèmes POSIX).

Via le navigateur web il est possible :
  * d'obtenir une console réseau (similaire à la console de l'IDE arduino)
  * d'obtenir une vue des variables (int/float/char ``*``) de la carte avec possibilité de les modifier, via une saisie avec clavier ou une checkbox. A cette vue peuvent être ajoutés des boutons qui, s'ils sont cliqués, émettent une (ou plusieurs) commandes à destination de la carte.
  
Via la connexion TCP il possible :
  * d'émettre toute commande à destination de tout autre carte/noeud (possédant un objet de type BoardView)
    * à partir d'une carte via la fonction tcpRequest fournie.
    * à partir d'un système POSIX, via la commande tcpRequest fournie

Pour fonctionner avec BoardView l'application doit intégrer la capacité à traiter des commandes simples. Ces commandes seront émises par d'autres programmes ou par le navigateur quand l'utilisateur interagira avec les widgets. 

Ce langage de commandes est complètement définissable par l'utilisateur et les exemples fournis plus bas implémentent tous un mini interpréteur de commandes. Ils peuvent vous servir de modèles dans un premier temps.

Cependant des constantes reviennent dans le langage de commandes :
  * donner accès en lecture à une de ses variables
  * idem mais en lecture/écriture
  * donner accès à une fonction (la capacité de déclencher la fonction).

Dans l'image ci-dessous une capacité intégrée (mais facultative) à BoardView : faire une redirection de commandes (et de leurs réponses) via un port série afin de rendre communiquants des arduinos qui ne le sont pas à la base : le d1-mini ne sert que de pont vers un arduino (nano ici). Décrit dans l'exemple n°2.

<p align="center">
<img src="https://raw.githubusercontent.com/supercc-arduino/BoardView/master/websock.png" width="800"> 
</p>

## Note importante sur les exemples

> ⚠️ **Warning**: Les #define suivants sont supposés être dans le fichier ``boardViewPrefs.h`` :

``` c
#define SSID "wifi-ssid"
#define PASSWORD "wifi-password"
```

Il vous faudra soit les définir dans le fichier soit les définir directement dans votre programme.

## Installation

Pré requis pour faire fonctionner les exemples :

  * Avoir installé la chaîne de compilation pour votre Esp (voir par exemple [ici](https://github.com/esp8266/Arduino))
  * Avoir installé la bibliothèque "Web Sockets" de Markus Sattler (version 2.\*) (via le menu "Outils -> Gérer les bibliothèques" par exemple pour l'IDE Arduino).
  * Avoir téléchargé et installé le ZIP de BoardView (bouton "Clone or download" en haut de cette page) et ensuite, pour l'IDE Arduino "Croquis -> Inclure une bibliothèque -> -> Ajouter la bibliothèque .ZIP"  (plus de détails [ici](https://www.robot-maker.com/ouvrages/tuto-arduino/bibliotheque-arduino/)).

Aucun matériel nécessaire pour les exemples autre qu'une carte esp8266 (et nano + 4 fils si test du mode "redirect").

## La vue

La vue de la carte est une page web ou les variables (de la carte) peuvent être consultées et/ou modifiées et disposant éventuellement de boutons émettant des commandes. 

  * Label : pour visualiser uniquement un couple varName=value
  * Entry : comme Label mais si on clic dessus un popup permet de modifier la valeur.
  * CheckBox : permet de lancer des commandes si activée/désactivé
  * Button : permet de démarrer l'exécution de fonctions.
  
Un aperçu de la vue de l'exemple n°1 :

<p align="center">
<img src="https://raw.githubusercontent.com/supercc-arduino/BoardView/master/view-chrono.png" width="200"> 
</center>
 
 Les widgets disponibles sont minimalistes de même que les possibilités de mise en forme de la page html. En échange la composition d'une vue est triviale (au niveau de code à fournir) :
 
```c++
	boardView.addLabel("chrono");
	boardView.addCheckBox("startStop", "startStop=0", "startStop=1");
	boardView.addButton("Clear", "clear");	
	
```
 
## Le langage de commandes

BoardView est conçu pour des échanges à base de commandes. Une commande est de la forme :

``cmd arg1 arg2 ... argN``

Une forme particulière de commande est la commande de modification de variable qui à la forme :

``varName=value``

Ou (à cause du signe = difficilement accessible à partir d'un clavier virtuel de smartphone)

``varname value``

Le nœud destinataire d'une commande construira la réponse, également sous la forme d'une unique ligne, qu'il retournera à l'émetteur de la commande. Cela peut être un résultat, ou juste l'information que la commande s'est correctement déroulée ("ok") ou pas ("erreur : code ou message...")

La première (et éventuellement la seule) commande que doit être capable d'interpréter un noeud est la commande :

``dump``

Très simple (pas d'argument) mais indispensable car en réponse le noeud communique tous les couples ``varName=value`` des variables qu'il souhaite rendre visibles en lecture seule (exemple de réponse : "var1=23.2; otherVar=8; again=1").

Si le nœud veut rendre possible la modification de certaines de ses variables alors il doit interpréter les messages de la forme :

``varname=value # (on rappelle que l'espace peut remplacer '=')``

C'est tout pour la théorie, place à la pratique ;-)

## Exemples d&apos;interactions via le navigateur

### Exemple 1.1 : Un chronomètre.

Pour ce premier exemple nous utiliserons une petite application de type chronomètre destinée à tester l'interactivité ( web socket esp8266).

On souhaite plus précisément voir la valeur du chronomètre (chrono de type float), pouvoir démarrer/arrêter le chronomètre (variable startStop) et enfin disposer d'un bouton clear.  

Sachant cela on peut définir les commandes du protocole :

  * dump : récupérer les valeurs des variables. Réponse de la forme : "chrono=XXX; startStop=XXX"
  * On veut pouvoir modifier ``startStop`` donc il faudra interpréter les commandes de la forme ``chrono=...`` et  `` startStop=...``
  * on veut une commande ``clear`` arrêtant et réinitialisant le compteur. 

Voila ce que cela donne en  C avec les [fonctions utilitaires](BoardViewProto.h) :

```c++
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
	 
	else if(matchAndAssignInt(request, "startStop", &startStop)) strcpy(response, "ok");
		
	// Autres commandes
	
	else if(matchCmd(request,"clear")) { startStop=0, chrono=0; response="ok"; }
	
	// Si la commande n'est pas reconnue, message d'erreur
	
	else { ret=1; sprintf(response, "Error : bad request : %s", request); }
	
	return ret; 
}
```

Le code [complet de l'exemple](examples/ChronoD1Mini/ChronoD1Mini.ino). Adaptez le ssid et le mot de passe !

Une fois le programme téléversé, ouvrez la console, passez la vitesse à 115200 bauds, après redémarrage de la carte elle doit afficher l'IP attribuée à votre carte par le point d'accès (ssid/password) : ``Wifi : IP  addr : 192.168.1.X``

Dans la barre de saisie de votre navigateur copiez/collez l'IP. Vous atterrissez sur la page principale proposant une console (réseau) et la vue définie.

<p align="center">
<img src="https://raw.githubusercontent.com/supercc-arduino/BoardView/master/chronos.png" width="640"> 
</p>

### Exemple 1.2 : Le chronomètre déporté sur une nano

BoardView peut également servir de pont entre un projet arduino sans capacité wifi mais implémentant le protocole de commandes définit plus haut et le réseau wifi. Le lien physique entre la carte arduino et le module esp8288 disposant de l'objet boardView est la liaison série (Serial) configurée à une vitesse de communication raisonnablement haute (115200 bauds).

Schéma des branchements, RX arduino vers TX ESP et TX arduino vers RX ESP. Adaptez les niveaux de tension entre les 2 cartes si nécessaire. Plusieurs blogueurs (exemple [ici](https://www.ba0sh1.com/blog/2016/08/03/is-esp8266-io-really-5v-tolerant/)), ou vendeurs (exemple [ici](https://protosupplies.com/product/esp8266-d1-mini-v2-esp-12f-wifi-module/)) indiquent que l'esp8266 est assez tolérant au 5V mais a vous de voir...

L'idée est simple : on sépare le code en 2 parties

  1. [Code sur la nano](examples/ChronoNano/ChronoNano.ino) : la gestion des capteurs/actionneurs + interpréteur de commandes

  2. [Code sur la d1-mini](examples/ChronoD1Mini/ChronoD1Mini.ino) (juste en mode "pont" web socket <-> Serial) : création d'un objet BoardView, définition de la vue démarrage en mode "redirection".

Remarques :
  * débranchez le lien série unissant les 2 cartes quand vous téléversez sur l'une ou l'autre.
  * la liaison série étant utilisée des 2 cotés vous ne pouvez plus vous en servir pour déboguer.
  * l'alimentation par l'USB du PC reste possible. 
  
### Exemple 1.3 : Un asservissement tout ou rien

L'asservissement [tout ou rien](https://fr.wikipedia.org/wiki/Tout_ou_rien) ([bang-bang](https://en.wikipedia.org/wiki/Bang%E2%80%93bang_control) en anglais) à le mérite de la simplicité.

Une petite mise en situation : supposons que je souhaite réguler le taux d'humidité de ma buanderie qui à la fâcheuse tendance à grimper lorsque j'y mets mon linge à sécher en hiver. Du coup mes cartons se ramollissent.... . Pour l'éviter je souhaite piloter, via un relai, un extracteur qui doit se déclencher dès que l'humidité atteint un certain seuil. Dans ce genre d'asservissement, pour éviter que les oscillations du capteur autour du seuil ne provoquent de nombreuses activations/désactivation du relais il est courant de définir une "marge" faisant que l'état du relais n'est pas changé par le système de régulation si la valeur est dans l'intervalle [seuil-marge, seuil+marge]. J'anticipe la suite en imposant que la régulation soit également conditionnée par l'état d'une variable onOff (int).

Un programme C possible sans considération "réseau" :

```c
float hum;               // variable reflétant l'état du capteur d'humidité.
float threshold=80;      // seuil au dessus duquel l'extraction est souhaité.
float margin=2;          // marge autour du seul pour éviter les 
			// oscillations trop rapides
			
int relay;               // variable reflétant l'état du relais (extracteur).    
int onOff;               // variable indiquant si la régulation doit être 
                         // activée : activée si 1, désactivée si 0.

if(hum < threshold-margin) {
	if(onOff) relay=0;
}
else if(hum > threshold+margin) {
	if(onOff) relay=1;
}
```  

Avec le réseau j'aimerai une vue dans laquelle je peux :
  * voir la valeur de ``hum``
  * voir et éventuellement modifier les variables ``threshold`` et ``margin``
  * voir et éventuellement modifier ``onOff`` et ``relay`` mais avec des checkboxs
  
 Du coup 
   * avec onOff je choisi le mode manuel ou le mode régulé
   * en mode manuel je pilote le relais avec la checkbox
 
Comme dans les exemples précédent le langage de commandes découle des opérations à effectuer :

```c
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

```

La description de la vue est :

```C
	boardView.addLabel("hum");
	boardView.addEntry("threshold");
	boardView.addEntry("margin");
	
    	boardView.addCheckBox("relay", "relay=0", "relay=1");
	boardView.addCheckBox("onOff", "onOff=0; relay=0;", "onOff=1");

```

<p align="center">
<img src="https://raw.githubusercontent.com/supercc-arduino/BoardView/master/bangbang.png" width="200"> 
</p>


Le code [complet de l'exemple](examples/BangBangD1Mini/BangBangD1Mini.ino). Adaptez le ssid et le mot de passe !

Une fois le programme téléversé, ouvrez la console, passez la vitesse à 115200 bauds, après redémarrage de la carte elle doit afficher l'IP attribuée à votre carte par le point d'accès (ssid/password) : ``Wifi : IP  addr : 192.168.1.X``

Dans la barre de saisie de votre navigateur copiez/collez l'IP. Vous atterrissez sur la page principale proposant une console (réseau) et la vue définie.

Remarques : 
  * les modifications ne survivent pas au reboot de la carte. A vous de voir par exemple à les sauvegarder dans l'EEPROM.
  * Il est possible de coller une liste de commandes à un bouton ou une checkbox
  
## FAQ

  * Peut-on contrôler ses cartes par d'Internet ?
  
  Rien ne l'empêche, mais c'est un problème de routage, à vous de configurer le nécessaire sur votre routeur/box.
  
  
