#ifndef TRI_H
#define TRI_H

#include <stdint.h>
#include <stdio.h>

#define NOMBRE_ALGORITHMES 6
#define NOMBRE_CONFIGURATIONS 5
#define NOMBRE_CAS 8
#define NOMBRE_TAILLES 10
#define TAILLE_MAXIMALE 1000000

typedef enum { SELECTION, BULLES, INSERTION, RAPIDE, FUSION, TAS } Algorithme;
typedef enum {
    ALEATOIRE, CROISSANT, DECROISSANT, PRESQUE_TRIE, VALEURS_REPETEES,
    QUICK_EQUILIBRE, QUICK_ALEATOIRE, QUICK_EGAUX
} Configuration;

typedef struct { uint64_t comparaisons, mouvements; } Operations;
typedef struct {
    int repetitions;
    int limite_lente;
    int taille_max;
} Parametres;
typedef struct {
    int effectue[NOMBRE_ALGORITHMES];
    double minimum[NOMBRE_ALGORITHMES], moyenne[NOMBRE_ALGORITHMES];
    Operations operations[NOMBRE_ALGORITHMES];
    uint32_t graine_entree, graine_pivot;
} ResultatExperience;

extern const int TAILLES[NOMBRE_TAILLES];
const char *nom_algorithme(Algorithme algorithme);
const char *fichier_algorithme(Algorithme algorithme);
const char *nom_configuration(Configuration configuration);
const char *fichier_configuration(Configuration configuration);
int algorithme_eligible(Algorithme algorithme, Configuration configuration,
                       int taille, int limite);
uint32_t nombre_aleatoire(uint32_t *etat);
uint32_t graine_experience(uint32_t graine, Configuration configuration,
                          int taille, int repetition, int pivot);
int generer_tableau(int *tableau, int taille, Configuration configuration,
                   uint32_t graine);
int tableau_est_trie(const int *tableau, int taille);
void appliquer_tri(Algorithme algorithme, int *tableau, int *tampon,
                   int taille, int pivot_milieu);
Operations compter_tri(Algorithme algorithme, int *tableau, int *tampon,
                       int taille, int pivot_milieu);
int executer_experience(int taille, Configuration configuration,
                       const Parametres *parametres, uint32_t graine,
                       FILE *mesures, ResultatExperience *resultat, int progression);

#endif