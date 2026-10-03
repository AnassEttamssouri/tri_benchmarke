#include "graphique.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

static int chemin(char *out, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int n = vsnprintf(out, size, format, args);
    va_end(args);
    return n >= 0 && (size_t)n < size;
}
static int assurer_dossier(const char *path)
{
    if (CreateDirectoryA(path, NULL)) return 1;
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
}
static int fichier_existe(const char *path)
{
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}
static FILE *ouvrir_sortie(const DossierExperience *d, const char *relative)
{
    char path[LONGUEUR_CHEMIN];
    if (!chemin(path, sizeof path, "%s/%s", d->resultats, relative)) return NULL;
    FILE *f = fopen(path, "w");
    if (!f) perror(path);
    return f;
}
static int fermer_sortie(FILE *f)
{
    int ok = !ferror(f);
    if (fclose(f) != 0) ok = 0;
    return ok;
}

int creer_dossier_experience(DossierExperience *d, const Parametres *p, uint32_t seed, int choice)
{
    time_t now = time(NULL);
    struct tm *date = localtime(&now);
    char base[24], path[LONGUEUR_CHEMIN];
    if (!date || !strftime(base, sizeof base, "%Y%m%d_%H%M%S", date) ||
        !assurer_dossier("resultats") || !assurer_dossier("graphiques")) return 0;
    int created = 0;
    for (int i = 0; i < 1000; ++i) {
        if (i == 0) snprintf(d->identifiant, sizeof d->identifiant, "%s", base);
        else snprintf(d->identifiant, sizeof d->identifiant, "%s_%03d", base, i);
        if (!chemin(d->resultats, sizeof d->resultats, "resultats/%s", d->identifiant) ||
            !chemin(d->graphiques, sizeof d->graphiques, "graphiques/%s", d->identifiant)) return 0;
        if (CreateDirectoryA(d->resultats, NULL)) { created = 1; break; }
        if (GetLastError() != ERROR_ALREADY_EXISTS) return 0;
    }
    if (!created || !assurer_dossier(d->graphiques)) return 0;
    const char *folders[] = {"benchmark", "cases"};
    for (int i = 0; i < 2; ++i) {
        if (!chemin(path, sizeof path, "%s/%s", d->resultats, folders[i]) || !assurer_dossier(path)) return 0;
    }
    const char *graphs[] = {"algorithm_benchmark", "cases", "configurations"};
    for (int i = 0; i < 3; ++i) {
        if (!chemin(path, sizeof path, "%s/%s", d->graphiques, graphs[i]) || !assurer_dossier(path)) return 0;
    }
    d->taille_max = p->taille_max;
    FILE *f = ouvrir_sortie(d, "informations.txt");
    if (!f) return 0;
    fprintf(f, "Application : tri_benchmark\nIdentifiant : %s\nGraine : %u\n", d->identifiant, (unsigned)seed);
    fprintf(f, "Repetitions : %d\nLimite lente : %d\nTaille maximale : %d\nChoix : %d\n",
            p->repetitions, p->limite_lente, p->taille_max, choice);
    fprintf(f, "Tailles :");
    for (int i = 0; i < NOMBRE_TAILLES && TAILLES[i] <= p->taille_max; ++i) fprintf(f, " %d", TAILLES[i]);
    fprintf(f, "\nTemps : secondes; minimum et moyenne arithmetique.\n"
               "Compteurs : un passage separe sur la repetition 1 (pas une moyenne).\n"
               "Comparaisons : valeurs entre elles ou valeur/pivot.\n"
               "Mouvements : ecritures de tableaux, tampon fusion inclus; un swap vaut 3.\n"
               "Generation/copies/verification exclues des temps et compteurs.\n"
               "Entrees : xorshift32, valeurs aleatoires de 0 a 1000000.\n"
               "Pivots : srand(graine_pivot), rand() du GCC utilise pour compiler.\n"
               "Graines de chaque repetition dans mesures.csv; algorithme de derivation dans tri.c.\n"
               "non_mesure : test ignore au-dela de la limite lente.\n");
    return fermer_sortie(f);
}

FILE *ouvrir_mesures(const DossierExperience *d)
{
    FILE *f = ouvrir_sortie(d, "mesures.csv");
    if (f && fprintf(f, "configuration;taille;repetition;algorithme;graine_entree;graine_pivot;temps_s\n") < 0) {
        fclose(f); return NULL;
    }
    return f;
}

static const LigneResultat *trouver(const LigneResultat *rows, int count, int n, Configuration c)
{
    for (int i = 0; i < count; ++i)
        if (rows[i].taille == n && rows[i].configuration == c) return &rows[i];
    return NULL;
}
static int configuration_presente(const LigneResultat *rows, int count, Configuration c)
{
    for (int i = 0; i < count; ++i) if (rows[i].configuration == c) return 1;
    return 0;
}
static void ecrire_valeur(FILE *f, const LigneResultat *row, int a)
{
    if (row && row->resultat.effectue[a]) fprintf(f, " %.12f", row->resultat.minimum[a]);
    else fprintf(f, " non_mesure");
}
static int ecrire_configuration(const DossierExperience *d, const LigneResultat *rows,
                                int count, Configuration c)
{
    char relative[128];
    snprintf(relative, sizeof relative, "%s.csv", fichier_configuration(c));
    FILE *f = ouvrir_sortie(d, relative);
    if (!f) return 0;
    fprintf(f, "taille");
    for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) fprintf(f, ";%s_min_s;%s_moyenne_s", fichier_algorithme((Algorithme)a), fichier_algorithme((Algorithme)a));
    fputc('\n', f);
    for (int i = 0; i < count; ++i) {
        if (rows[i].configuration != c) continue;
        const ResultatExperience *r = &rows[i].resultat;
        fprintf(f, "%d", rows[i].taille);
        for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
            if (r->effectue[a]) fprintf(f, ";%.12f;%.12f", r->minimum[a], r->moyenne[a]);
            else fputs(";non_mesure;non_mesure", f);
        }
        fputc('\n', f);
    }
    return fermer_sortie(f);
}

int enregistrer_resultats(const DossierExperience *d, const LigneResultat *rows, int count)
{
    FILE *f = ouvrir_sortie(d, "resume.csv");
    if (!f) return 0;
    fprintf(f, "configuration;taille;algorithme;statut;minimum_s;moyenne_s\n");
    for (int i = 0; i < count; ++i) {
        for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
            if (rows[i].configuration >= QUICK_EQUILIBRE && a != RAPIDE) continue;
            fprintf(f, "%s;%d;%s;", fichier_configuration(rows[i].configuration), rows[i].taille, fichier_algorithme((Algorithme)a));
            if (rows[i].resultat.effectue[a]) fprintf(f, "mesure;%.12f;%.12f\n", rows[i].resultat.minimum[a], rows[i].resultat.moyenne[a]);
            else fprintf(f, "non_mesure;non_mesure;non_mesure\n");
        }
    }
    if (!fermer_sortie(f)) return 0;
    f = ouvrir_sortie(d, "operations.csv");
    if (!f) return 0;
    fprintf(f, "configuration;taille;algorithme;statut;repetition_representative;graine_entree;graine_pivot;comparaisons;mouvements\n");
    for (int i = 0; i < count; ++i) {
        const ResultatExperience *r = &rows[i].resultat;
        for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
            if (rows[i].configuration >= QUICK_EQUILIBRE && a != RAPIDE) continue;
            fprintf(f, "%s;%d;%s;", fichier_configuration(rows[i].configuration), rows[i].taille, fichier_algorithme((Algorithme)a));
            if (r->effectue[a]) fprintf(f, "mesure;1;%u;%u;%" PRIu64 ";%" PRIu64 "\n", (unsigned)r->graine_entree,
                                       (unsigned)r->graine_pivot, r->operations[a].comparaisons, r->operations[a].mouvements);
            else fputs("non_mesure;non_mesure;non_mesure;non_mesure;non_mesure;non_mesure\n", f);
        }
    }
    if (!fermer_sortie(f)) return 0;
    for (int c = 0; c < NOMBRE_CONFIGURATIONS; ++c)
        if (configuration_presente(rows, count, (Configuration)c) && !ecrire_configuration(d, rows, count, (Configuration)c)) return 0;
    if (configuration_presente(rows, count, ALEATOIRE)) {
        f = ouvrir_sortie(d, "benchmark/res.dat");
        if (!f) return 0;
        fputs("# Size Selection_Sort Bubble_Sort Insertion_Sort Quick_Sort Merge_Sort Heap_Sort (minimum seconds)\n", f);
        for (int i = 0; i < count; ++i) if (rows[i].configuration == ALEATOIRE) {
            fprintf(f, "%d", rows[i].taille);
            for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) ecrire_valeur(f, &rows[i], a);
            fputc('\n', f);
        }
        if (!fermer_sortie(f)) return 0;
    }
    const Configuration order[] = { CROISSANT, ALEATOIRE, DECROISSANT, PRESQUE_TRIE, VALEURS_REPETEES };
    for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
        int have_general = 0;
        for (int i = 0; i < count; ++i) if (rows[i].configuration < QUICK_EQUILIBRE) have_general = 1;
        if (!have_general) break;
        char relative[128];
        snprintf(relative, sizeof relative, "cases/%s_cases.dat", fichier_algorithme((Algorithme)a));
        f = ouvrir_sortie(d, relative);
        if (!f) return 0;
        fputs("# N Sorted Random Reversed Nearly_Sorted Ten_Repeated_Values (minimum seconds)\n", f);
        for (int s = 0; s < NOMBRE_TAILLES && TAILLES[s] <= d->taille_max; ++s) {
            fprintf(f, "%d", TAILLES[s]);
            for (int c = 0; c < NOMBRE_CONFIGURATIONS; ++c) ecrire_valeur(f, trouver(rows, count, TAILLES[s], order[c]), a);
            fputc('\n', f);
        }
        if (!fermer_sortie(f)) return 0;
    }
    if (configuration_presente(rows, count, QUICK_EQUILIBRE)) {
        f = ouvrir_sortie(d, "cases/quick_sort_best_avg_worst_cases.dat");
        if (!f) return 0;
        fputs("# N Balanced_Partitions Random All_Equal (minimum seconds)\n", f);
        for (int s = 0; s < NOMBRE_TAILLES && TAILLES[s] <= d->taille_max; ++s) {
            fprintf(f, "%d", TAILLES[s]);
            for (int c = QUICK_EQUILIBRE; c <= QUICK_EGAUX; ++c) ecrire_valeur(f, trouver(rows, count, TAILLES[s], (Configuration)c), RAPIDE);
            fputc('\n', f);
        }
        if (!fermer_sortie(f)) return 0;
    }
    return 1;
}

int enregistrer_dernier_dossier(const DossierExperience *d)
{
    FILE *f = fopen("resultats/dernier_run.txt", "w");
    if (!f) return 0;
    fprintf(f, "%s\n", d->identifiant);
    return fermer_sortie(f);
}
int charger_dernier_dossier(DossierExperience *d)
{
    FILE *f = fopen("resultats/dernier_run.txt", "r");
    if (!f) return 0;
    if (!fgets(d->identifiant, sizeof d->identifiant, f)) { fclose(f); return 0; }
    fclose(f);
    d->identifiant[strcspn(d->identifiant, "\r\n")] = 0;
    if (!d->identifiant[0]) return 0;
    for (int i = 0; d->identifiant[i]; ++i)
        if (!isdigit((unsigned char)d->identifiant[i]) && d->identifiant[i] != '_') return 0;
    if (!chemin(d->resultats, sizeof d->resultats, "resultats/%s", d->identifiant) ||
        !chemin(d->graphiques, sizeof d->graphiques, "graphiques/%s", d->identifiant)) return 0;
    d->taille_max = TAILLE_MAXIMALE;
    return 1;
}

static int trouver_gnuplot(char *out, size_t capacity)
{
    const char *custom = getenv("GNUPLOT_EXE");
    if (custom && custom[0]) return fichier_existe(custom) && chemin(out, capacity, "%s", custom);
    DWORD n = SearchPathA(NULL, "gnuplot.exe", NULL, (DWORD)capacity, out, NULL);
    if (n > 0 && n < capacity) return 1;
    const char *places[] = {"C:/Program Files/gnuplot/bin/gnuplot.exe", "C:/Program Files (x86)/gnuplot/bin/gnuplot.exe"};
    for (int i = 0; i < 2; ++i) if (fichier_existe(places[i])) return chemin(out, capacity, "%s", places[i]);
    return 0;
}

/* Determine which series actually contain measurements, avoiding empty plots. */
static int analyser_donnees(const char *path, int csv, int count, int small, int *available)
{
    memset(available, 0, (size_t)count * sizeof *available);
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    char line[2048];
    if (!fgets(line, sizeof line, f)) { fclose(f); return 0; }
    while (fgets(line, sizeof line, f)) {
        char *field = strtok(line, csv ? ";\r\n" : " \t\r\n");
        if (!field || (small && atoi(field) > 5000)) continue;
        for (int i = 0; i < count; ++i) {
            field = strtok(NULL, csv ? ";\r\n" : " \t\r\n");
            if (field && strcmp(field, "non_mesure") != 0) available[i] = 1;
            if (csv) strtok(NULL, ";\r\n"); /* average follows minimum */
        }
    }
    int ok = !ferror(f);
    fclose(f);
    return ok;
}

static int creer_script(const DossierExperience *d, const char *data_relative,
                        const char *graph_relative, const char *title, int csv,
                        int count, const char *const *labels, int first, int small,
                        char *script_path)
{
    char data[LONGUEUR_CHEMIN], png[LONGUEUR_CHEMIN];
    if (!chemin(data, sizeof data, "%s/%s", d->resultats, data_relative) ||
        !chemin(script_path, LONGUEUR_CHEMIN, "%s/%s.plt", d->graphiques, graph_relative) ||
        !chemin(png, sizeof png, "%s/%s.png", d->graphiques, graph_relative)) return -1;
    if (!fichier_existe(data)) return 0;
    int available[NOMBRE_ALGORITHMES];
    if (!analyser_donnees(data, csv, count, small, available)) return -1;
    int series = 0;
    for (int i = first; i < count; ++i) series += available[i];
    if (!series) return 0;
    FILE *f = fopen(script_path, "w");
    if (!f) return -1;
    fprintf(f, "set encoding utf8\nset terminal pngcairo size 1024,768 font 'Arial,10'\n");
    fprintf(f, "set output '%s'\nset datafile missing 'non_mesure'\n", png);
    if (csv) fputs("set datafile separator ';'\n", f);
    fprintf(f, "set title '%s'\nset xlabel 'Taille du tableau (N)'\nset ylabel 'Temps minimum (secondes)'\nset grid\nset yrange [0:*]\n", title);
    if (small) fputs("set xrange [0:5000]\n", f);
    fputs("plot ", f);
    int printed = 0;
    for (int i = first; i < count; ++i) {
        if (!available[i]) continue;
        if (printed++) fputs(", ", f);
        fprintf(f, "'%s' %susing 1:%d title '%s' with linespoints", data, csv ? "every ::1 " : "", csv ? 2 + 2 * i : 2 + i, labels[i]);
    }
    fputs("\nunset output\n", f);
    return fermer_sortie(f) ? 1 : -1;
}

/* Launch without cmd.exe: quoted installation paths and no visible helper window. */
static int executer_gnuplot(const char *exe, const char *script)
{
    char command[LONGUEUR_CHEMIN * 2 + 16];
    if (!chemin(command, sizeof command, "\"%s\" \"%s\"", exe, script)) return 0;
    STARTUPINFOA start;
    PROCESS_INFORMATION process;
    memset(&start, 0, sizeof start);
    start.cb = sizeof start;
    if (!CreateProcessA(exe, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &start, &process)) return 0;
    DWORD exit_code = 1;
    WaitForSingleObject(process.hProcess, INFINITE);
    GetExitCodeProcess(process.hProcess, &exit_code);
    CloseHandle(process.hProcess); CloseHandle(process.hThread);
    return exit_code == 0;
}

int generer_graphiques(const DossierExperience *d)
{
    char scripts[40][LONGUEUR_CHEMIN], relative[128], title[256];
    int count = 0, result;
    const char *algo_labels[NOMBRE_ALGORITHMES];
    for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) algo_labels[a] = nom_algorithme((Algorithme)a);
    const char *case_labels[] = {"Tableau trie", "Tableau aleatoire", "Tableau inverse", "Tableau presque trie", "Dix valeurs repetees"};
    const char *quick_labels[] = {"Partitions equilibrees (pivot milieu)", "Tableau aleatoire (pivot aleatoire)", "Toutes les valeurs egales (pivot aleatoire)"};
    result = creer_script(d, "benchmark/res.dat", "algorithm_benchmark/algorithm_benchmark", "Sorting Algorithm Performance", 0, NOMBRE_ALGORITHMES, algo_labels, 0, 0, scripts[count]);
    if (result < 0) return 0;
    count += result;
    for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
        char data[128];
        snprintf(data, sizeof data, "cases/%s_cases.dat", fichier_algorithme((Algorithme)a));
        snprintf(relative, sizeof relative, "cases/%s_cases", fichier_algorithme((Algorithme)a));
        snprintf(title, sizeof title, "%s - Comparaison des configurations", nom_algorithme((Algorithme)a));
        result = creer_script(d, data, relative, title, 0, 5, case_labels, 0, 0, scripts[count]);
        if (result < 0) return 0;
        count += result;
    }
    result = creer_script(d, "cases/quick_sort_best_avg_worst_cases.dat", "cases/quick_sort_best_avg_worst_cases", "Tri rapide - Partitions equilibrees, aleatoire et valeurs egales", 0, 3, quick_labels, 0, 0, scripts[count]);
    if (result < 0) return 0;
    count += result;
    for (int c = 0; c < NOMBRE_CONFIGURATIONS; ++c) {
        char data[128];
        snprintf(data, sizeof data, "%s.csv", fichier_configuration((Configuration)c));
        for (int view = 0; view < 3; ++view) {
            const char *suffix = view == 1 ? "_rapides" : view == 2 ? "_petites_tailles" : "";
            snprintf(relative, sizeof relative, "configurations/%s%s", fichier_configuration((Configuration)c), suffix);
            snprintf(title, sizeof title, "Comparaison - %s%s", nom_configuration((Configuration)c), view == 1 ? " - rapide, fusion et tas" : view == 2 ? " - tailles jusqu a 5000" : "");
            result = creer_script(d, data, relative, title, 1, NOMBRE_ALGORITHMES, algo_labels, view == 1 ? RAPIDE : 0, view == 2, scripts[count]);
            if (result < 0) return 0;
            count += result;
        }
    }
    if (!count) { puts("Aucune donnee disponible pour les graphiques."); return 0; }
    char exe[LONGUEUR_CHEMIN];
    if (!trouver_gnuplot(exe, sizeof exe)) {
        puts("Gnuplot introuvable. Mesures et scripts .plt conserves; regeneration disponible au menu.");
        return 0;
    }
    int ok = 1;
    for (int i = 0; i < count; ++i) if (!executer_gnuplot(exe, scripts[i])) {
        fprintf(stderr, "Echec de Gnuplot : %s. Script conserve.\n", scripts[i]); ok = 0;
    }
    if (ok) printf("%d graphiques PNG crees dans %s.\n", count, d->graphiques);
    return ok;
}