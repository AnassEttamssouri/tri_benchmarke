#ifndef BENCHMARK_H
#define BENCHMARK_H
#include "tri.h"
#define NOMBRE_CONFIGURATIONS 5
#define NOMBRE_CAS 8
#define NOMBRE_TAILLES 10
#define TAILLE_MAXIMALE 1000000
typedef enum {
    ALEATOIRE, CROISSANT, DECROISSANT, PRESQUE_TRIE, VALEURS_REPETEES,
    QUICK_EQUILIBRE, QUICK_ALEATOIRE, QUICK_EGAUX
} Configuration;
typedef struct { int repetitions, limite_lente, taille_max; } Parametres;
typedef struct {
    int effectue[NOMBRE_ALGORITHMES];
    double minimum[NOMBRE_ALGORITHMES], moyenne[NOMBRE_ALGORITHMES];
    Operations operations[NOMBRE_ALGORITHMES];
    uint32_t graine_entree, graine_pivot;
} ResultatExperience;
typedef struct { int taille; Configuration configuration; ResultatExperience resultat; } LigneResultat;
typedef struct {
    Configuration configuration;
    Algorithme algorithme;
    int taille, repetition;
    uint32_t graine_entree, graine_pivot;
    double secondes;
} MesureBenchmark;
typedef enum { PROGRESSION_EXCLU, PROGRESSION_MESURE, PROGRESSION_COMPTE } PhaseProgression;
typedef void (*ProgressionBenchmark)(PhaseProgression, Algorithme, Configuration,
                                    int, int, const Parametres *);
/* A NULL measurement requests the end-of-experiment flush. */
typedef int (*EnregistrerMesure)(const MesureBenchmark *, void *);
extern const int TAILLES[NOMBRE_TAILLES];
const char *nom_configuration(Configuration configuration);
const char *fichier_configuration(Configuration configuration);
uint32_t nombre_aleatoire(uint32_t *etat);
uint32_t graine_campagne(void);
uint32_t graine_experience(uint32_t graine, Configuration configuration,
                          int taille, int repetition, int pivot);
int generer_tableau(int *tableau, int taille, Configuration configuration, uint32_t graine);
int algorithme_eligible(Algorithme algorithme, Configuration configuration, int taille, int limite);
const LigneResultat *trouver(const LigneResultat *lignes, int nombre, int taille, Configuration configuration);
int configuration_presente(const LigneResultat *lignes, int nombre, Configuration configuration);
int executer_experience(int taille, Configuration configuration, const Parametres *parametres,
                       uint32_t graine, EnregistrerMesure enregistrer, void *contexte,
                       ResultatExperience *resultat, ProgressionBenchmark progression);
int executer_campagne(const Parametres *parametres, int choix, uint32_t graine,
                     EnregistrerMesure enregistrer, void *contexte, LigneResultat *lignes,
                     int *nombre, ProgressionBenchmark progression,
                     void (*rapporter)(const LigneResultat *));
#endif
