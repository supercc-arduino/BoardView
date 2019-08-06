# BoardView

_ En cours de construction - pas de code dispo pour l'instant _

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

Le noeud destinataire d'une commande construira la réponse, également sous la forme d'une unique ligne, qu'il retournera à l'émetteur de la commande. Cela peut être un résultat, ou juste l'information que la commande s'est correctement déroulé ("ok") ou pas ("erreur : code ou message...")

La première (et éventuellement la seule) commande que doit être capable d'interpréte un noeud est la commande :

''dump''

Très simple (pas d'argument) mais indispensable car en réponse le noeud communique tous les couples ``varName=value`` des variables qu'il souhaite rendre visibles (exemple de réponse : "var1=23.2; otherVar=8; again=1").

Si le noeud veut rendre possible la modification de certaines de ses variables alors il doit interpréter les messages de la forme :

''varname=value'' (on rappelle que l'espace prut remplacer '=')

C'est tout ;-)

== Fonctions utilitaires

Sous arduino, écrire ne serait ce qu'un flottant dans une chaîne de caractères peut s'avérer assez sportif (le "%f" n'est pas présent pour sscanf/sprintf sur les petites architectures ). Des fonctions utilitaires de codage/décodage des messages sont fournies par les fichiers BoardViewProto.h/BoardViewProto.cpp pour simplifier le travail du programmeur. Elles seronts utilisées dans les exemples.

// Construction d'une chaine de caractères par concaténation de couple varName=value
char *addInt(char *dst, char *name, int value);
char *addFloat(char *dst, char *name, float value);

// Fonctions d'aide au décodage des messages reçus

int matchCmd(char *line, char *cmd);
int matchAndAssignFloat(char *line, char *varName, float *varAddr);
int matchAndAssignInt(char *line, char *varName, int *varAddr);

// Fonction récupérant des lignes d'un flux pour les traiter comme des commandes

int readAndParseLines(Stream &stream, int (*parseRequest)(char *request, char *response, unsigned len)

== Mise en oeuvre 

=== Premier exemple ===

Un chronomètre sur un esp d1-mini visualisé et contrôlé par le navigateur. Pas trop d'intérêt réel, mais assez visuel pour constater l'intérêt de l'utilisation d'une web socket ;-)

Outre la variable compteur (de type float pour l'exemple), la variable active (de type int) permet de définir si le compteur s'incrémente ou non. On souhaite pouvoir faire une rémise à zéro du compteur en cliquant sur un bouton Clear. On souhaite également que la led de la carte soit allumée si et si seulement le compteur est actif.

Le code de base sans considération "réseaux"

```


```



