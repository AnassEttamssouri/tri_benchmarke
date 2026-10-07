#include "benchmark.h"
#include "complexite.h"
#include "resultats.h"
#include "graphique.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
    int count;
    const char *const *lines = descriptions_theoriques(&count);
    for (int i = 0; i < count; ++i) puts(lines[i]);
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
static void afficher_progression(PhaseProgression phase, Algorithme a, Configuration c,
                                int n, int repetition, const Parametres *p)
{
    if (phase == PROGRESSION_EXCLU)
        printf("  %s : non_mesure (limite %d)\n", nom_algorithme(a), p->limite_lente);
    else if (phase == PROGRESSION_COMPTE)
        puts("    Comptage separe (hors chronometrage)...");
    else printf("  %s | %s | N=%d | repetition %d/%d\n", nom_algorithme(a),
                nom_configuration(c), n, repetition, p->repetitions);
    fflush(stdout);
}
static void afficher_estimation(const char *label, const EstimationCroissance *estimate)
{
    printf("    %-25s : %-10s | %d point(s)", label, nom_modele(estimate->modele), estimate->points);
    if (estimate->points >= 3 && isfinite(estimate->dispersion)) {
        printf(" | pente %.3f | dispersion %.3f", estimate->exposant, estimate->dispersion);
        if (estimate->modele == MODELE_INDETERMINE) printf(" | aucun modele retenu");
        else if (estimate->dispersion > DISPERSION_INCERTAINE) printf(" | incertain");
    } else printf(" | au moins 3 valeurs positives necessaires");
    putchar('\n');
}
static int analyser_dossier(const DossierExperience *d, const LigneResultat *rows, int count, int display)
{
    AnalyseComplexite analyses[NOMBRE_ANALYSES_MAX];
    int total = analyser_complexites(rows, count, analyses, NOMBRE_ANALYSES_MAX);
    int saved = enregistrer_analyse(d, analyses, total);
    if (!saved) puts("Analyse non exportee; les mesures restent disponibles.");
    if (display) {
        printf("\n===== Analyse experimentale du run %s =====\n", d->identifiant);
        puts("Operations = comparaisons + mouvements, passage representatif (pas une moyenne).");
        puts("Temps = minimum en secondes, tailles N >= 1000. Modeles : n, n log n, n^2.");
        puts("Dispersion > 0.25 : incertain; > 0.60 : aucun modele retenu.");
        puts("Une estimation sur ces tailles n'est pas une preuve de complexite ou de cas moyen.");
        puts("La verification exacte porte uniquement sur les comparaisons et les formules applicables.");
        for (int i = 0; i < total; ++i) {
            const AnalyseComplexite *a = &analyses[i];
            if (!i || a->configuration != analyses[i - 1].configuration)
                printf("\n[%s]\n", nom_configuration(a->configuration));
            printf("  %s : %s\n", nom_algorithme(a->algorithme), a->reference);
            afficher_estimation("Operations representatives", &a->operations);
            afficher_estimation("Temps minimum (secondes)", &a->temps);
            if (a->points_exacts) printf("    Comparaisons exactes : %s (%d tailles, %d ecarts)\n",
                                        nom_verification(a->verification), a->points_exacts, a->ecarts_exacts);
        }
    }
    if (saved) printf("Analyse sauvegardee : %s/complexite.csv\n", d->resultats);
    return saved;
}
static void menu_complexites(void)
{
    for (;;) {
        puts("\n1. Complexites theoriques\n2. Analyse du dernier run\n0. Retour");
        int choice = lire_entier("Votre choix : ", 0, 2);
        if (choice <= 0) return;
        if (choice == 1) afficher_complexites();
        else {
            DossierExperience d;
            LigneResultat rows[NOMBRE_CAS * NOMBRE_TAILLES];
            int count;
            if (!charger_dernier_dossier(&d)) puts("Aucun dernier test disponible.");
            else if (charger_resultats_analyse(&d, rows, &count)) analyser_dossier(&d, rows, count, 1);
        }
    }
}
static int lancer_tests(const Parametres *p, int choice)
{
    DossierExperience d;
    LigneResultat rows[NOMBRE_CAS * NOMBRE_TAILLES];
    uint32_t seed = graine_campagne();
    if (!creer_dossier_experience(&d, p, seed, choice)) {
        fputs("Impossible de creer le dossier d'experience.\n", stderr); return 0;
    }
    FILE *mesures = ouvrir_mesures(&d);
    if (!mesures) return 0;
    printf("\nRun %s | graine %u | repetitions %d | limite lente %d\n",
           d.identifiant, (unsigned)seed, p->repetitions, p->limite_lente);
    puts("Temps : secondes. Les tris lents et le rapide sur doublons sont limites.");
    int count = 0;
    int ok = executer_campagne(p, choice, seed, enregistrer_mesure, mesures, rows,
                              &count, afficher_progression, afficher_resultat);
    if (fclose(mesures) != 0) { fputs("Echec de fermeture des mesures.\n", stderr); ok = 0; }
    if (!ok) {
        fprintf(stderr, "Mesures deja terminees conservees dans %s; ce run ne remplace pas le dernier run valide.\n", d.resultats);
        return 0;
    }
    if (!enregistrer_resultats(&d, rows, count) || !enregistrer_dernier_dossier(&d)) {
        fputs("Impossible d'enregistrer les resultats.\n", stderr); return 0;
    }
    printf("\nResultats : %s\nGraphiques et scripts : %s\n", d.resultats, d.graphiques);
    analyser_dossier(&d, rows, count, 0);
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

static int ouvrir_selection_ui(const DossierExperience *d, int c, int a, unsigned selection, int small)
{
    DemandeCourbe demande = {COURBE_COMPARAISON, (Configuration)c,
                            a < 0 ? SELECTION : (Algorithme)a, selection, small};
    if (c == NOMBRE_CONFIGURATIONS && a >= 0) demande.type = COURBE_CONFIGURATIONS;
    else if (c == QUICK_EQUILIBRE) demande.type = COURBE_RAPIDE;
    return afficher_courbe(d, &demande);
}
static int mesure_presente(const LigneResultat *rows, int count, int c, int a)
{
    for (int i = 0; i < count; ++i)
        if (rows[i].configuration < QUICK_EQUILIBRE &&
            (c < 0 || rows[i].configuration == (Configuration)c) &&
            rows[i].resultat.effectue[a]) return 1;
    return 0;
}

/* Returns a configuration, NOMBRE_CONFIGURATIONS for all cases, or -1 to cancel. */
static int choisir_configuration_courbe(const LigneResultat *rows, int count, int a,
                                       int (*read_int)(const char *, int, int))
{
    int choices[NOMBRE_CONFIGURATIONS + 1], total = 0;
    for (int c = 0; c < NOMBRE_CONFIGURATIONS; ++c)
        if (configuration_presente(rows, count, (Configuration)c) &&
            (a < 0 || mesure_presente(rows, count, c, a))) {
            choices[total++] = c;
            printf("%d. %s\n", total, nom_configuration((Configuration)c));
        }
    if (a >= 0 && total) {
        choices[total++] = NOMBRE_CONFIGURATIONS;
        printf("%d. Toutes les configurations\n", total);
    }
    if (!total) { puts("Aucune configuration mesuree disponible pour ce choix."); return -1; }
    puts("0. Retour");
    int choice = read_int("Configuration : ", 0, total);
    return choice <= 0 ? -1 : choices[choice - 1];
}

static unsigned choisir_algorithmes_courbe(int number, int (*read_int)(const char *, int, int))
{
    unsigned selection = 0;
    for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) printf("%d. %s\n", a + 1, nom_algorithme((Algorithme)a));
    puts("0. Retour");
    for (int selected = 0; selected < number;) {
        printf("Algorithme %d/%d :\n", selected + 1, number);
        int choice = read_int("Votre choix : ", 0, NOMBRE_ALGORITHMES);
        if (choice <= 0) return 0;
        unsigned bit = 1u << (choice - 1);
        if (selection & bit) { puts("Algorithme deja choisi. Choisissez-en un autre."); continue; }
        selection |= bit;
        ++selected;
    }
    return selection;
}

static int afficher_courbes(const DossierExperience *d, int (*read_int)(const char *, int, int))
{
    for (;;) {
        printf("\n===== Courbes du dernier run : %s =====\n", d->identifiant);
        puts("1. Un algorithme");
        puts("2. Tous les algorithmes");
        puts("3. Comparer deux algorithmes");
        puts("4. Comparer plusieurs algorithmes");
        puts("5. Experience du tri rapide");
        puts("6. Regenerer les graphiques");
        puts("0. Retour");
        int choice = read_int("Votre choix : ", 0, 6);
        if (choice <= 0) return 1;
        if (choice == 6) { generer_graphiques(d); continue; }
        LigneResultat rows[NOMBRE_CAS * NOMBRE_TAILLES];
        int count;
        if (!charger_resume(d, rows, &count)) continue;
        if (choice == 5) {
            if (!configuration_presente(rows, count, QUICK_EQUILIBRE)) {
                puts("Experience du tri rapide absente de ce run."); continue;
            }
            ouvrir_selection_ui(d, QUICK_EQUILIBRE, -1, 1u << RAPIDE, 0);
            continue;
        }
        int a = -1;
        unsigned selection = (1u << NOMBRE_ALGORITHMES) - 1u;
        if (choice == 1) {
            selection = choisir_algorithmes_courbe(1, read_int);
            if (!selection) continue;
            for (a = 0; !(selection & (1u << a)); ++a) {}
        }
        int c = choisir_configuration_courbe(rows, count, a, read_int);
        if (c < 0) continue;
        if (choice == 3 || choice == 4) {
            int number = 2;
            if (choice == 4) {
                do {
                    number = read_int("Nombre d'algorithmes (3 a 6, 0 pour retour) : ", 0, NOMBRE_ALGORITHMES);
                    if (number <= 0) break;
                    if (number < 3) puts("Choisissez entre 3 et 6 algorithmes.");
                } while (number > 0 && number < 3);
                if (number <= 0) continue;
            }
            selection = choisir_algorithmes_courbe(number, read_int);
            if (!selection) continue;
        }
        int small = 0;
        if (c < NOMBRE_CONFIGURATIONS) {
            puts("1. Toutes les tailles disponibles");
            puts("2. Tailles jusqu'a 5000");
            puts("0. Retour");
            int view = read_int("Vue : ", 0, 2);
            if (view <= 0) continue;
            small = view == 2;
            for (int algorithm = 0; algorithm < NOMBRE_ALGORITHMES; ++algorithm) {
                if (!(selection & (1u << algorithm))) continue;
                int skipped = 0;
                for (int i = 0; i < count; ++i)
                    if (rows[i].configuration == (Configuration)c && (!small || rows[i].taille <= 5000) &&
                        !rows[i].resultat.effectue[algorithm]) {
                        if (!skipped++) printf("%s : non_mesure aux tailles", nom_algorithme((Algorithme)algorithm));
                        printf(" %d", rows[i].taille);
                    }
                if (skipped) putchar('\n');
            }
        }
        ouvrir_selection_ui(d, c, a, selection, small);
    }
}

static int afficher_histogramme(const DossierExperience *d, int (*read_int)(const char *, int, int))
{
    LigneResultat rows[NOMBRE_CAS * NOMBRE_TAILLES];
    int count, choices[NOMBRE_CONFIGURATIONS + 1], total = 0;
    if (!charger_resume(d, rows, &count)) return 0;
    printf("Histogramme du dernier run complet : %s (mesures enregistrees)\n", d->identifiant);
    for (int c = 0; c <= NOMBRE_CONFIGURATIONS; ++c) {
        Configuration config = c == NOMBRE_CONFIGURATIONS ? QUICK_EQUILIBRE : (Configuration)c;
        if (configuration_presente(rows, count, config)) {
            choices[total++] = config;
            printf("%d. %s\n", total, c == NOMBRE_CONFIGURATIONS ? "Experience tri rapide (les trois cas)" : nom_configuration(config));
        }
    }
    int choice = read_int("Configuration : ", 1, total);
    if (choice < 0) return 0;
    Configuration config = (Configuration)choices[choice - 1];
    int sizes[NOMBRE_TAILLES], available = 0;
    for (int s = 0; s < NOMBRE_TAILLES; ++s) if (trouver(rows, count, TAILLES[s], config)) {
        sizes[available++] = TAILLES[s];
        printf("%d. %d elements\n", available, TAILLES[s]);
    }
    choice = read_int("Taille de l'histogramme : ", 1, available);
    if (choice < 0) return 0;
    return generer_histogramme(d, config, sizes[choice - 1], 1);
}

int main(void)
{
    Parametres p = {20, 20000, TAILLE_MAXIMALE};
    initialiser_console();
    setlocale(LC_ALL, ""); setlocale(LC_NUMERIC, "C");
    for (;;) {
        puts("\n===== Etude comparative des algorithmes de tri =====");
        puts("1. Tous les tests");
        puts("2. Test au choix");
        printf("3. Parametres (repetitions %d, limite lente %d, taille max %d)\n", p.repetitions, p.limite_lente, p.taille_max);
        puts("4. Complexites et analyse");
        puts("5. Afficher les courbes");
        puts("6. Afficher un histogramme");
        puts("7. Quitter");
        int choice = lire_entier("Votre choix : ", 1, 7);
        if (choice < 0 || choice == 7) { puts("Au revoir."); break; }
        switch (choice) {
        case 1: lancer_tests(&p, -1); break;
        case 2: choisir_test(&p); break;
        case 3: regler_tests(&p); break;
        case 4: menu_complexites(); break;
        case 5: {
            DossierExperience d;
            if (charger_dernier_dossier(&d)) afficher_courbes(&d, lire_entier);
            else puts("Aucun dernier test disponible.");
            break;
        }
        case 6: {
            DossierExperience d;
            if (charger_dernier_dossier(&d)) afficher_histogramme(&d, lire_entier);
            else puts("Aucun dernier test disponible.");
            break;
        }
        }
    }
    return 0;
}
