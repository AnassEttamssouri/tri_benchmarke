#ifndef TRI_H
#define TRI_H
#include <stdint.h>
#define NOMBRE_ALGORITHMES 6
typedef enum { SELECTION, BULLES, INSERTION, RAPIDE, FUSION, TAS } Algorithme;
typedef struct { uint64_t comparaisons, mouvements; } Operations;
const char *nom_algorithme(Algorithme algorithme);
const char *fichier_algorithme(Algorithme algorithme);
void initialiser_pivots(uint32_t graine);
int tableau_est_trie(const int *tableau, int taille);
void appliquer_tri(Algorithme algorithme, int *tableau, int *tampon, int taille, int pivot_milieu);
Operations compter_tri(Algorithme algorithme, int *tableau, int *tampon, int taille, int pivot_milieu);
#endif
