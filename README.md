# BoardView

BoardView est une classe C++ pour ESP8266 permettant de communiquer avec vos cartes arduinos à l'aide d'un navigateur Web (façon interractive) ou via une connexion TCP (pour l'automatisation du pilotage/controle des cartes avec d'autres programmes (scripts, Ruby, ...). 

Via le navigateur web il est possible :
  * d'obtenir une console réseau (similaire à la console de l'IDE arduino)
  * d'obtenir une vue des variables (int/float) de la carte avec possibiliter de les modifier, via une saisie avec clavier ou une checkbox. A cette vue peuvent être rajouter des boutons qui, s'ils sont cliqués, émettent uen (ou plusieurs) commandes à la carte.
  
Via la connexion TCP il est possible d'émettre toute commande à .

Pour fonctionner avec BoardView l'application doit intégrer le codage/décodage du protocole. Ce protocole sera décrit plus bas mais est un protocoles à base de commandes que l'utilisateur (via le navigateur en cliquant sur des boutons) ou les programmes (via la connexion TCP) pourront émettre.

Les cartes possédant un objet de type BoardView peuvent émettre des commandes aux autres cartes également équipées.

Dans l'image ci-dessous le d1-mini de sert que de pont vers un arduino (nano ici).

![Screenshot](websock.jpg) 

