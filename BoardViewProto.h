#ifndef BOARD_VIEW_PROTO_H
#define BOARD_VIEW_PROTO_H

#include <Arduino.h>

/*
 * Conversion float vers chaîne (compatible arduino).
 *   char * s : la destination du résultat
 *   int len  : la longueur max de la réponse
 *   float f  : le float à convertir dans s
 *   int prec : le nombre de chiffre après la virgule souhaité
 * 
 * Valeur de retour : le paramètre s.
 * 
 * */
 
char *f2s(char *s, int len, float f, int prec);


/*
 * Convertion float vers chaîne (compatible arduino).
 *   char * s : la chaine contenant la représentation du float.
 * 
 * Valeur de retour : le float résultat de la conversion.
 * 
 * */

float s2f(char *s) ;

 
/*
 * Fonctions de construction d'une chaine de caractères par 
 * concaténation de couples varName=value séparés par des ';'
 * 
 * Ajoute un entier.
 *   char *dst  : la chaine destination.
 *   int len    : la longueur allouée pour la chaine destination
 *   char *name : nom de la variable à ajouter
 *   int value  : valeur à ajouter 
 * 
 * Valeur de retour : le paramètre dst.
 * 
 * */
 
char *addInt(char *dst, unsigned len, char *name, int value); 


/*
 * Ajoute un float.
 *   char *dst   : la chaine destination.
 *   int len     : la longueur allouée pour la chaine destination
 *   char *name  : nom de la variable à ajouter
 *   float value : valeur à ajouter 
 * 
 * Valeur de retour : le paramètre dst.
 * 
 * */
 
char *addFloat(char *dst, unsigned len, char *name, float value, int prec);

/*
 * Ajoute un chaîne de caractères.
 *   char *dst   : la chaine destination.
 *   int len     : la longueur allouée pour la chaine destination
 *   char *name  : nom de la variable à ajouter
 *   char *s :   : la chaîne à ajouter 
 * 
 * Valeur de retour : le paramètre dst.
 * 
 * */
 
 
char *addStr(char *dst, unsigned len, char *name, char *s);

/*
 *  Fonctions d'aide au décodage des messages reçus
 * 
 * Retourne 1 (vrai) si la ligne correspond à la commande, 0 sinon.
 *  char *line : la ligne
 *  char *cmd  : la commande
 * 
 * Valeur de retour : 1 (vrai) si la ligne correspond, 0 sinon.
 *
 * */
 
int matchCmd(char *line, char *cmd); 

/*
 * Affectation de float.
 * Si la ligne est de la forme varname=value 
 * alors l'affectation est faite.
 * 
 *  char *line     : la ligne
 *  char *varName  : le nom de la variable à affecter
 *  float *varAddr : l'adresse de la variable à affecter.
 * 
 * Valeur de retour : 1 (vrai) si la ligne correspondait, 0 sinon.
 *
 * */

int matchAndAssignFloat(char *line, char *varName, float *varAddr); 


/*
 * Affectation de int.
 * Si la ligne est de la forme varname=value 
 * alors l'affectation est faite.
 * 
 *  char *line    : la ligne
 *  char *varName : le nom de la variable à affecter
 *  int *varAddr  : l'adresse de la variable à affecter.

 * Valeur de retour : 1 (vrai) si la ligne correspondait, 0 sinon.
 * */
int matchAndAssignInt(char *line, char *varName, int *varAddr);


/*
 * Affectation de chaîne.
 * Si la ligne est de la forme varname=value 
 * alors l'affectation est faite.
 * 
 *  char *line    : la ligne
 *  char *varName : le nom de la variable à affecter
 *  char *varAddr : l'adresse de la variable à affecter
 *  int len       : la taille de la chaîne destination.

 * Valeur de retour : 1 (vrai) si la ligne correspondait, 0 sinon.
 * */
 
int matchAndAssignStr(char *line, char *varName, char *varAddr, int len);


/*
 * Fonction récupérant des lignes d'un flux pour les traiter comme des commandes.
 *
 *   Stream &stream : le flux (Stream) (Serial est un flux possible).
 *   parseRequest : la fonction de traitement des requêtes.
 *   int len      : la longuer max d'une ligne (requête ou réponse).
 * */  
 
int readAndParseLines(Stream &stream, int (*parseRequest)(char *request, char *response, int len), int len);

#endif
