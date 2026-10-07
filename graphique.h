#ifndef GRAPHIQUE_H
#define GRAPHIQUE_H
#include "resultats.h"
typedef enum { COURBE_COMPARAISON, COURBE_CONFIGURATIONS, COURBE_RAPIDE } TypeCourbe;
typedef struct {
    TypeCourbe type;
    Configuration configuration;
    Algorithme algorithme;
    unsigned selection;
    int petites_tailles;
} DemandeCourbe;
int generer_graphiques(const DossierExperience *dossier);
int afficher_courbe(const DossierExperience *dossier, const DemandeCourbe *demande);
int generer_histogramme(const DossierExperience *dossier, Configuration configuration,
                       int taille, int ouvrir_image);
#endif
