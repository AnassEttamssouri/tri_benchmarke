#ifndef GRAPHIQUE_H
#define GRAPHIQUE_H

#include "tri.h"

#define LONGUEUR_CHEMIN 512
typedef struct {
    char identifiant[40];
    char resultats[LONGUEUR_CHEMIN];
    char graphiques[LONGUEUR_CHEMIN];
    int taille_max;
} DossierExperience;
typedef struct {
    int taille;
    Configuration configuration;
    ResultatExperience resultat;
} LigneResultat;

int creer_dossier_experience(DossierExperience *dossier, const Parametres *parametres,
                            uint32_t graine, int choix);
FILE *ouvrir_mesures(const DossierExperience *dossier);
int enregistrer_resultats(const DossierExperience *dossier,
                          const LigneResultat *lignes, int nombre);
int enregistrer_dernier_dossier(const DossierExperience *dossier);
int charger_dernier_dossier(DossierExperience *dossier);
int generer_graphiques(const DossierExperience *dossier);

#endif