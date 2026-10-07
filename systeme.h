#ifndef SYSTEME_H
#define SYSTEME_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#define LONGUEUR_CHEMIN 512
typedef enum { DOSSIER_ERREUR, DOSSIER_CREE, DOSSIER_EXISTANT } EtatDossier;
typedef struct { char cpu[256]; unsigned long processeurs; int64_t frequence; } InformationsMachine;
void initialiser_console(void);
void informations_machine(InformationsMachine *machine);
int chrono_disponible(void);
int chrono_instant(int64_t *instant);
double chrono_secondes(int64_t debut, int64_t fin);
int chemin(char *sortie, size_t taille, const char *format, ...);
EtatDossier creer_dossier(const char *path);
int assurer_dossier(const char *path);
int fichier_existe(const char *path);
int fermer_sortie(FILE *fichier);
int remplacer_fichier(const char *source, const char *destination);
int trouver_gnuplot(char *sortie, size_t capacite);
int executer_gnuplot(const char *executable, const char *script);
int ouvrir_png(const char *path);
/* Missing folders contain no files; visit all files, even after a failure. */
int parcourir_fichiers(const char *dossier, const char *motif,
                      int (*visiter)(const char *, void *), void *contexte, int *nombre);
#endif
