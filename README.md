# BoardView

__ En cours de construction - pas de code dispo pour l'instant __

BoardView est une classe C++ pour ESP8266 permettant de communiquer avec vos cartes arduinos à l'aide d'un navigateur Web (façon interractive) ou via une connexion TCP (pour l'automatisation du pilotage/controle des cartes avec d'autres programmes (scripts, Ruby, ...). 

Via le navigateur web il est possible :
  * d'obtenir une console réseau (similaire à la console de l'IDE arduino)
  * d'obtenir une vue des variables (int/float) de la carte avec possibiliter de les modifier, via une saisie avec clavier ou une checkbox. A cette vue peuvent être ajoutés des boutons qui, s'ils sont cliqués, émettent une (ou plusieurs) commandes à destination de la carte.
  
Via la connexion TCP il est possible d'émettre toute commande à destinaton de tout autre carte/noeud (possédant un objet de type BoardView).

Pour fonctionner avec BoardView l'application doit intégrer le codage/décodage du protocole. Ce protocole sera décrit plus bas mais est un protocole orienté commandes (un mini shell Unix ;-) que l'utilisateur (via le navigateur en cliquant sur des boutons) ou les programmes (via la connexion TCP) pourront émettres.

Dans l'image ci-dessous le d1-mini de sert que de pont vers un arduino (nano ici).

![Screenshot](websock.jpg) 


## Le protocole de communication 

A base de commandes donc. Une commande est de la forme :

``cmd arg1 arg2 ... argN``

Une forme particulière de commande est la commande de modification de variable qui à la forme :

``varName=value``

Ou (à cause du signe = difficilement accessible à partir d'un clavier virtuel de smartphone)

``varname value``

Une autre particularité liée aux claviers virtuels qui ont tendance à mettre en majuscule la première lettre de la commande, est qu'une commande verra automatiquement sa première lettre passée en minuscule.

Un effet de bord est qu'il ne sera pas possible d'affecter la valeur aux variables dont le nom commence par une majuscule, mais qui met une majuscule à ses noms de variables ?

La première (et éventuellement la seule) commande que doit être capable d'interpréte un noeud est la commande :

''dump''

Très simple (pas d'argument) mais indispensable car en réponse le noeud communique tous les couples ``varName=value`` des variables qu'il souhaite rendre visibles.

Si le noeud veut rendre possible la modification de certaines de ses variables alors il doit interpréter les messages de la forme :

''varname=value''

C'est tout ;-)




