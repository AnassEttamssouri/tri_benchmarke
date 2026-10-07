#ifndef COMPLEXITE_H
#define COMPLEXITE_H
#include "benchmark.h"
#define NOMBRE_ANALYSES_MAX (NOMBRE_CONFIGURATIONS * NOMBRE_ALGORITHMES + 3)
#define DISPERSION_INCERTAINE 0.25
#define DISPERSION_MAXIMALE 0.60
typedef enum { MODELE_LINEAIRE, MODELE_NLOGN, MODELE_QUADRATIQUE, MODELE_INDETERMINE } ModeleCroissance;
typedef enum { EXACT_NON_APPLICABLE, EXACT_VERIFIE, EXACT_ECART } VerificationExacte;
typedef struct { int points; ModeleCroissance modele; double exposant, dispersion; } EstimationCroissance;
typedef struct {
    Configuration configuration;
    Algorithme algorithme;
    const char *reference;
    EstimationCroissance operations, temps;
    VerificationExacte verification;
    int points_exacts, ecarts_exacts;
} AnalyseComplexite;
const char *nom_modele(ModeleCroissance modele);
const char *nom_verification(VerificationExacte verification);
const char *reference_complexite(Algorithme algorithme, Configuration configuration);
const char *const *descriptions_theoriques(int *nombre);
EstimationCroissance estimer_croissance(const double *tailles, const double *valeurs, int nombre);
int analyser_complexites(const LigneResultat *lignes, int nombre,
                        AnalyseComplexite *analyses, int capacite);
#endif
