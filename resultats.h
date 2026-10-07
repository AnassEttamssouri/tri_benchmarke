#ifndef RESULTATS_H
#define RESULTATS_H
#include "benchmark.h"
#include "complexite.h"
#include "systeme.h"
typedef struct {
    char identifiant[40];
    char resultats[LONGUEUR_CHEMIN], graphiques[LONGUEUR_CHEMIN];
    int taille_max;
} DossierExperience;
int creer_dossier_experience(DossierExperience *dossier, const Parametres *parametres,
                            uint32_t graine, int choix);
FILE *ouvrir_mesures(const DossierExperience *dossier);
int enregistrer_mesure(const MesureBenchmark *mesure, void *fichier);
int enregistrer_resultats(const DossierExperience *dossier, const LigneResultat *lignes, int nombre);
int enregistrer_dernier_dossier(const DossierExperience *dossier);
int charger_dernier_dossier(DossierExperience *dossier);
int charger_resume(const DossierExperience *dossier, LigneResultat *lignes, int *nombre);
int charger_resultats_analyse(const DossierExperience *dossier, LigneResultat *lignes, int *nombre);
int enregistrer_analyse(const DossierExperience *dossier, const AnalyseComplexite *analyses, int nombre);
#endif
