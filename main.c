#include "tri.h"
#include "graphique.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

static int lire_entier(const char *question, int minimum, int maximum)
{
    char ligne[128], *fin;
    for (;;) {
        printf("%s", question); fflush(stdout);
        if (!fgets(ligne, sizeof ligne, stdin)) return -1;
        if (!strchr(ligne, '\n') && !feof(stdin)) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            puts("Saisie trop longue."); continue;
        }
        errno = 0;
        long value = strtol(ligne, &fin, 10);
        while (isspace((unsigned char)*fin)) ++fin;
        if (!errno && fin != ligne && !*fin && value >= minimum && value <= maximum) return (int)value;
        printf("Veuillez saisir un entier entre %d et %d.\n", minimum, maximum);
    }
}
static int choisir_taille(void)
{
    puts("Tailles testees depuis 100 jusqu'a la taille maximale choisie :");
    for (int i = 0; i < NOMBRE_TAILLES; ++i) printf("%d. %d elements\n", i + 1, TAILLES[i]);
    int choice = lire_entier("Taille maximale : ", 1, NOMBRE_TAILLES);
    return choice < 0 ? -1 : TAILLES[choice - 1];
}
static void afficher_complexites(void)
{
    puts("\nComplexites theoriques (meilleur / moyen / pire) :");
    puts("Bulles avec arret anticipe : O(n) / O(n^2) / O(n^2)");
    puts("Selection                 : O(n^2) / O(n^2) / O(n^2)");
    puts("Insertion                 : O(n) / O(n^2) / O(n^2)");
    puts("Rapide, pivot aleatoire   : O(n log n) / O(n log n) / O(n^2)");
    puts("Fusion et tas             : O(n log n) dans les trois cas (borne superieure).");
    puts("Partition rapide en deux groupes : les doublons peuvent provoquer O(n^2).");
    puts("Valeurs theoriques; les mesures en secondes et les compteurs sont reels.");
}
static void afficher_resultat(const LigneResultat *row)
{
    printf("\n%s, N=%d (secondes) :\n", nom_configuration(row->configuration), row->taille);
    for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
        if (row->configuration >= QUICK_EQUILIBRE && a != RAPIDE) continue;
        if (!row->resultat.effectue[a]) {
            printf("  %-20s : non_mesure (limite lente)\n", nom_algorithme((Algorithme)a)); continue;
        }
        printf("  %-20s : min %.9f s | moyenne %.9f s | comparaisons %" PRIu64 " | mouvements %" PRIu64 "\n",
               nom_algorithme((Algorithme)a), row->resultat.minimum[a], row->resultat.moyenne[a],
               row->resultat.operations[a].comparaisons, row->resultat.operations[a].mouvements);
    }
    puts("  Compteurs : un passage representatif sur l'entree de la repetition 1.");
}
static int lancer_tests(const Parametres *p, int choice)
{
    DossierExperience d;
    LigneResultat rows[NOMBRE_CAS * NOMBRE_TAILLES];
    uint32_t seed = (uint32_t)time(NULL);
    if (!seed) seed = 1;
    if (!creer_dossier_experience(&d, p, seed, choice)) {
        fputs("Impossible de creer le dossier d'experience.\n", stderr); return 0;
    }
    FILE *mesures = ouvrir_mesures(&d);
    if (!mesures) return 0;
    printf("\nRun %s | graine %u | repetitions %d | limite lente %d\n",
           d.identifiant, (unsigned)seed, p->repetitions, p->limite_lente);
    puts("Temps : secondes. Les tris lents et le rapide sur doublons sont limites.");
    int count = 0, ok = 1;
    for (int c = 0; c < NOMBRE_CAS && ok; ++c) {
        if (choice >= 0 && ((choice < NOMBRE_CONFIGURATIONS && c != choice) ||
            (choice == NOMBRE_CONFIGURATIONS && c < QUICK_EQUILIBRE))) continue;
        for (int s = 0; s < NOMBRE_TAILLES && TAILLES[s] <= p->taille_max; ++s) {
            LigneResultat *row = &rows[count];
            row->taille = TAILLES[s]; row->configuration = (Configuration)c;
            if (!executer_experience(row->taille, row->configuration, p, seed, mesures, &row->resultat, 1)) {
                fprintf(stderr, "Test interrompu : %s, N=%d.\n", nom_configuration((Configuration)c), row->taille);
                ok = 0; break;
            }
            ++count;
            afficher_resultat(row);
        }
    }
    if (fclose(mesures) != 0) { fputs("Echec de fermeture des mesures.\n", stderr); ok = 0; }
    if (!ok) {
        fprintf(stderr, "Mesures deja terminees conservees dans %s; ce run ne remplace pas le dernier run valide.\n", d.resultats);
        return 0;
    }
    if (!enregistrer_resultats(&d, rows, count) || !enregistrer_dernier_dossier(&d)) {
        fputs("Impossible d'enregistrer les resultats.\n", stderr); return 0;
    }
    printf("\nResultats : %s\nGraphiques et scripts : %s\n", d.resultats, d.graphiques);
    generer_graphiques(&d); /* plot failure never invalidates saved measurements */
    return 1;
}
static void regler_tests(Parametres *p)
{
    int r = lire_entier("Repetitions chronometrees (1 a 100) : ", 1, 100);
    if (r < 0) return;
    int limit = lire_entier("Limite des tests lents et rapide sur doublons (1 a 1000000) : ", 1, TAILLE_MAXIMALE);
    if (limit < 0) return;
    int maximum = choisir_taille();
    if (maximum < 0) return;
    *p = (Parametres){r, limit, maximum};
    puts("Parametres mis a jour pour cette session.");
}
static void choisir_test(const Parametres *p)
{
    for (int c = 0; c < NOMBRE_CONFIGURATIONS; ++c) printf("%d. %s\n", c + 1, nom_configuration((Configuration)c));
    puts("6. Experience rapide : partitions equilibrees / aleatoire / toutes valeurs egales");
    int choice = lire_entier("Configuration : ", 1, 6);
    if (choice < 0) return;
    int maximum = choisir_taille();
    if (maximum < 0) return;
    Parametres selected = *p;
    selected.taille_max = maximum;
    lancer_tests(&selected, choice - 1);
}

int main(void)
{
    Parametres p = {20, 20000, TAILLE_MAXIMALE};
    SetConsoleOutputCP(CP_UTF8); SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, ""); setlocale(LC_NUMERIC, "C");
    for (;;) {
        puts("\n===== Etude comparative des algorithmes de tri =====");
        puts("1. Lancer tous les tests");
        puts("2. Tester une configuration jusqu'a une taille maximale");
        printf("3. Regler les tests (repetitions %d, limite lente %d, taille max %d)\n", p.repetitions, p.limite_lente, p.taille_max);
        puts("4. Afficher les complexites theoriques");
        puts("5. Regenerer les graphiques du dernier test");
        puts("6. Quitter");
        int choice = lire_entier("Votre choix : ", 1, 6);
        if (choice < 0 || choice == 6) { puts("Au revoir."); break; }
        switch (choice) {
        case 1: lancer_tests(&p, -1); break;
        case 2: choisir_test(&p); break;
        case 3: regler_tests(&p); break;
        case 4: afficher_complexites(); break;
        case 5: {
            DossierExperience d;
            if (charger_dernier_dossier(&d)) generer_graphiques(&d);
            else puts("Aucun dernier test disponible.");
            break;
        }
        }
    }
    return 0;
}