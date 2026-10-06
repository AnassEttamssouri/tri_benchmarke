#include "graphique.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <shellapi.h>

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
               "Pivots : xorshift32-rejet-v1, etat separe; meme graine avant temps et comptage.\n"
               "Minimum chronometre : minimum des repetitions, pas meilleur cas theorique.\n"
               "Graines de chaque repetition dans mesures.csv; algorithme de derivation dans tri.c.\n"
               "non_mesure : test ignore au-dela de la limite lente.\n");
    SYSTEM_INFO machine;
    LARGE_INTEGER frequency;
    GetSystemInfo(&machine);
    const char *cpu = getenv("PROCESSOR_IDENTIFIER");
    fprintf(f, "Compilateur : GCC %s\nOptimisation : %s\nCPU : %s\nProcesseurs logiques : %lu\n",
            __VERSION__,
#ifdef __OPTIMIZE__
            "active (__OPTIMIZE__); build de livraison avec -O2",
#else
            "inactive",
#endif
            cpu ? cpu : "identifiant indisponible", (unsigned long)machine.dwNumberOfProcessors);
    if (QueryPerformanceFrequency(&frequency) && frequency.QuadPart > 0)
        fprintf(f, "Frequence du chronometre QPC : %lld Hz\n", (long long)frequency.QuadPart);
    else fputs("Frequence du chronometre QPC : indisponible\n", f);
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

static int creer_script_selection(const DossierExperience *d, const char *data_relative,
                        const char *graph_relative, const char *title, int csv,
                        int count, const char *const *labels, unsigned selection, int small,
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
    for (int i = 0; i < count; ++i) if (selection & (1u << i)) series += available[i];
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
    const char *colors[] = {"#d62728", "#ff7f0e", "#9467bd", "#1f77b4", "#2ca02c", "#8c564b"};
    for (int i = 0; i < count; ++i) {
        if (!(selection & (1u << i)) || !available[i]) continue;
        if (printed++) fputs(", ", f);
        fprintf(f, "'%s' %susing 1:%d title '%s' with linespoints lc rgb '%s'", data, csv ? "every ::1 " : "", csv ? 2 + 2 * i : 2 + i, labels[i], colors[i]);
    }
    fputs("\nunset output\n", f);
    return fermer_sortie(f) ? 1 : -1;
}
static int creer_script(const DossierExperience *d, const char *data_relative,
                        const char *graph_relative, const char *title, int csv,
                        int count, const char *const *labels, int first, int small,
                        char *script_path)
{
    unsigned selection = ((1u << count) - 1u) & ~((1u << first) - 1u);
    return creer_script_selection(d, data_relative, graph_relative, title, csv,
                                  count, labels, selection, small, script_path);
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
            if (c == ALEATOIRE && view == 0) continue; /* Global comparison is authoritative. */
            const char *suffix = view == 1 ? "_rapides" : view == 2 ? "_petites_tailles" : "";
            snprintf(relative, sizeof relative, "configurations/%s%s", fichier_configuration((Configuration)c), suffix);
            snprintf(title, sizeof title, "Comparaison - %s%s", nom_configuration((Configuration)c), view == 1 ? " - rapide, fusion et tas" : view == 2 ? " - tailles jusqu a 5000" : "");
            result = creer_script(d, data, relative, title, 1, NOMBRE_ALGORITHMES, algo_labels, view == 1 ? RAPIDE : 0, view == 2, scripts[count]);
            if (result < 0) return 0;
            count += result;
        }
    }
    char exe[LONGUEUR_CHEMIN];
    if (!trouver_gnuplot(exe, sizeof exe)) {
        puts("Gnuplot introuvable. Mesures et scripts .plt conserves; regeneration disponible au menu.");
        return 0;
    }
    int ok = 1;
    for (int i = 0; i < count; ++i) if (!executer_gnuplot(exe, scripts[i])) {
        fprintf(stderr, "Echec de Gnuplot : %s. Script conserve.\n", scripts[i]); ok = 0;
    }
    const char *folders[] = {"histogrammes", "comparaisons"};
    for (int folder = 0; folder < 2; ++folder) {
        char pattern[LONGUEUR_CHEMIN], script[LONGUEUR_CHEMIN];
        if (!chemin(pattern, sizeof pattern, "%s/%s/*.plt", d->graphiques, folders[folder])) return 0;
        WIN32_FIND_DATAA found;
        HANDLE search = FindFirstFileA(pattern, &found);
        if (search != INVALID_HANDLE_VALUE) {
            do {
                if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
                if (!chemin(script, sizeof script, "%s/%s/%s", d->graphiques, folders[folder], found.cFileName) ||
                    !executer_gnuplot(exe, script)) {
                    ok = 0; fprintf(stderr, "Echec de regeneration : %s\n", found.cFileName);
                }
                ++count;
            } while (FindNextFileA(search, &found));
            FindClose(search);
        }
    }
    if (!count) { puts("Aucune donnee disponible pour les graphiques."); return 0; }
    if (ok) printf("%d graphiques PNG crees dans %s.\n", count, d->graphiques);
    return ok;
}

/* Strictly read the unchanged summary format. Reject partial, duplicated or
   invalid records rather than drawing an apparently valid histogram. */
static int charger_resume(const DossierExperience *d, LigneResultat *rows, int *count)
{
    char path[LONGUEUR_CHEMIN], line[1024];
    int seen[NOMBRE_CAS * NOMBRE_TAILLES][NOMBRE_ALGORITHMES] = {{0}};
    *count = 0;
    if (!chemin(path, sizeof path, "%s/resume.csv", d->resultats)) return 0;
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "Resume introuvable : %s\n", path); return 0; }
    int ok = fgets(line, sizeof line, f) &&
        strcmp(line, "configuration;taille;algorithme;statut;minimum_s;moyenne_s\n") == 0;
    while (ok && fgets(line, sizeof line, f)) {
        if (!strchr(line, '\n') && !feof(f)) { ok = 0; break; }
        line[strcspn(line, "\r\n")] = 0;
        char *fields[6], *part = line;
        for (int i = 0; i < 6; ++i) {
            fields[i] = part;
            char *sep = strchr(part, ';');
            if ((i < 5 && !sep) || (i == 5 && sep)) { ok = 0; break; }
            if (sep) { *sep = 0; part = sep + 1; }
            if (!fields[i][0]) { ok = 0; break; }
        }
        if (!ok) break;
        int c = -1, a = -1, n = 0;
        for (int i = 0; i < NOMBRE_CAS; ++i)
            if (!strcmp(fields[0], fichier_configuration((Configuration)i))) c = i;
        for (int i = 0; i < NOMBRE_ALGORITHMES; ++i)
            if (!strcmp(fields[2], fichier_algorithme((Algorithme)i))) a = i;
        char *end;
        errno = 0;
        long size = strtol(fields[1], &end, 10);
        if (!errno && !*end)
            for (int i = 0; i < NOMBRE_TAILLES; ++i) if (size == TAILLES[i]) n = (int)size;
        if (c < 0 || a < 0 || !n || (c >= QUICK_EQUILIBRE && a != RAPIDE)) { ok = 0; break; }
        int r;
        for (r = 0; r < *count; ++r) if (rows[r].configuration == (Configuration)c && rows[r].taille == n) break;
        if (r == *count) {
            if (*count == NOMBRE_CAS * NOMBRE_TAILLES) { ok = 0; break; }
            memset(&rows[r], 0, sizeof rows[r]);
            rows[r].configuration = (Configuration)c; rows[r].taille = n;
            ++*count;
        }
        if (seen[r][a]++) { ok = 0; break; }
        if (!strcmp(fields[3], "non_mesure")) {
            if (strcmp(fields[4], "non_mesure") || strcmp(fields[5], "non_mesure")) ok = 0;
        } else if (!strcmp(fields[3], "mesure")) {
            double values[2];
            for (int i = 0; i < 2; ++i) {
                errno = 0;
                values[i] = strtod(fields[4 + i], &end);
                if (errno || *end || !isfinite(values[i]) || values[i] < 0) ok = 0;
            }
            if (!ok || values[0] > values[1] + 1e-12) { ok = 0; break; }
            rows[r].resultat.effectue[a] = 1;
            rows[r].resultat.minimum[a] = values[0];
            rows[r].resultat.moyenne[a] = values[1];
        } else ok = 0;
    }
    if (ferror(f) || !*count) ok = 0;
    fclose(f);
    for (int r = 0; ok && r < *count; ++r) {
        for (int a = 0; a < NOMBRE_ALGORITHMES; ++a)
            if ((rows[r].configuration < QUICK_EQUILIBRE || a == RAPIDE) && !seen[r][a]) ok = 0;
        if (rows[r].configuration >= QUICK_EQUILIBRE)
            for (int c = QUICK_EQUILIBRE; c <= QUICK_EGAUX; ++c)
                if (!trouver(rows, *count, rows[r].taille, (Configuration)c)) ok = 0;
    }
    if (!ok) fprintf(stderr, "Resume vide, incomplet ou malforme : %s\n", path);
    return ok;
}

static int ecrire_histogramme(const DossierExperience *d, const char *base,
                             const char *title, int count, const char *const *labels,
                             const int *measured, const double *minimum, const double *average,
                             char *script, char *png)
{
    char data[LONGUEUR_CHEMIN];
    if (!chemin(data, sizeof data, "%s/histogrammes/%s.dat", d->graphiques, base) ||
        !chemin(script, LONGUEUR_CHEMIN, "%s/histogrammes/%s.plt", d->graphiques, base) ||
        !chemin(png, LONGUEUR_CHEMIN, "%s/histogrammes/%s.png", d->graphiques, base)) return 0;
    FILE *f = fopen(data, "w");
    if (!f) { perror(data); return 0; }
    fputs("categorie;minimum_s;moyenne_s\n", f);
    double maximum = 0;
    for (int i = 0; i < count; ++i) {
        if (measured[i]) {
            fprintf(f, "%s;%.12f;%.12f\n", labels[i], minimum[i], average[i]);
            if (average[i] > maximum) maximum = average[i];
        } else fprintf(f, "%s (non mesure);non_mesure;non_mesure\n", labels[i]);
    }
    if (!fermer_sortie(f)) return 0;
    f = fopen(script, "w");
    if (!f) { perror(script); return 0; }
    fprintf(f, "set encoding utf8\nset terminal pngcairo size 1400,900 font 'Arial,12'\n"
               "set output '%s'\nset datafile separator ';'\nset datafile missing 'non_mesure'\n"
               "set title '%s'\nset ylabel 'Temps de tri (secondes)'\n"
               "set style data histograms\nset style histogram clustered gap 1\n"
               "set style fill solid 0.85 border -1\nset boxwidth 0.85\n"
               "set grid ytics\nset key outside top center horizontal\n"
               "set xtics rotate by -15\nset bmargin 7\nset yrange [0:%.12g]\n",
            png, title, maximum > 0 ? maximum * 1.55 : 1.0);
    fputs("set label 1 'Minimum des repetitions et moyenne arithmetique; aucune nouvelle mesure.' at graph 0.01,0.97 front\n", f);
    int skipped = 0;
    for (int i = 0; i < count; ++i) if (!measured[i]) {
        fprintf(f, "set label %d 'Non mesure : %s (limite lente)' at graph 0.01,%.3f front\n",
                2 + skipped, labels[i], 0.91 - skipped * 0.045);
        ++skipped;
    }
    fprintf(f, "plot '%s' every ::1 using 2:xticlabels(1) title 'Minimum (s)' lc rgb '#1f77b4', "
               "'' every ::1 using 3 title 'Moyenne (s)' lc rgb '#ff7f0e', "
               "'' every ::1 using ($0-0.17):2:(sprintf('%%.6g s',column(2))) with labels rotate by 90 left offset 0,1 notitle, "
               "'' every ::1 using ($0+0.17):3:(sprintf('%%.6g s',column(3))) with labels rotate by 90 left offset 0,1 notitle\n"
               "unset output\n", data);
    return fermer_sortie(f);
}

static int ouvrir_png(const char *png)
{
    char absolute[LONGUEUR_CHEMIN];
    DWORD n = GetFullPathNameA(png, sizeof absolute, absolute, NULL);
    if (!n || n >= sizeof absolute || !fichier_existe(absolute)) return 0;
    return (INT_PTR)ShellExecuteA(NULL, "open", absolute, NULL, NULL, SW_SHOWNORMAL) > 32;
}

/* Reuse saved PNGs, including when Gnuplot is unavailable. Only missing images
   need rendering; the saved measurements are never rewritten. */
static int montrer_courbe(const DossierExperience *d, const char *data,
                          const char *relative, const char *title, int csv,
                          int count, const char *const *labels, unsigned selection, int small)
{
    char png[LONGUEUR_CHEMIN], script[LONGUEUR_CHEMIN], folder[LONGUEUR_CHEMIN];
    if (!chemin(png, sizeof png, "%s/%s.png", d->graphiques, relative) ||
        !chemin(script, sizeof script, "%s/%s.plt", d->graphiques, relative)) return 0;
    printf("Courbe : %s\n", png);
    puts("Temps minimum en secondes. Les tests non mesures n'ont aucun point, jamais zero.");
    if (!fichier_existe(png)) {
        if (!chemin(folder, sizeof folder, "%s/%s", d->graphiques, relative)) return 0;
        char *slash = strrchr(folder, '/');
        if (!slash) return 0;
        *slash = 0;
        if (!assurer_dossier(d->graphiques) || !assurer_dossier(folder)) {
            fprintf(stderr, "Impossible de creer le dossier : %s\n", folder); return 0;
        }
        int result = creer_script_selection(d, data, relative, title, csv, count,
                                            labels, selection, small, script);
        if (result <= 0) {
            fprintf(stderr, "Donnees absentes, illisibles ou sans mesure pour cette vue : %s/%s\n",
                    d->resultats, data); return 0;
        }
        char exe[LONGUEUR_CHEMIN];
        printf("Script : %s\n", script);
        if (!trouver_gnuplot(exe, sizeof exe)) {
            puts("Gnuplot introuvable. Script conserve; utiliser Courbes > Regenerer les graphiques.");
            return 0;
        }
        if (!executer_gnuplot(exe, script) || !fichier_existe(png)) {
            fprintf(stderr, "Echec de creation du PNG. Script conserve : %s\n", script); return 0;
        }
    }
    if (!ouvrir_png(png)) printf("Visionneuse indisponible. Ouvrir manuellement : %s\n", png);
    return 1;
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

static int montrer_selection(const DossierExperience *d, int c, int a, unsigned selection, int small)
{
    char data[128], relative[128], title[256];
    const char *labels[NOMBRE_ALGORITHMES];
    int csv = 0, count;
    if (c == NOMBRE_CONFIGURATIONS && a >= 0) {
        const char *cases[] = {"Tableau trie", "Tableau aleatoire", "Tableau inverse", "Tableau presque trie", "Dix valeurs repetees"};
        count = NOMBRE_CONFIGURATIONS;
        for (int i = 0; i < count; ++i) labels[i] = cases[i];
        selection = (1u << count) - 1u;
        snprintf(data, sizeof data, "cases/%s_cases.dat", fichier_algorithme((Algorithme)a));
        snprintf(relative, sizeof relative, "cases/%s_cases", fichier_algorithme((Algorithme)a));
        snprintf(title, sizeof title, "%s - Comparaison des configurations", nom_algorithme((Algorithme)a));
    } else if (c == QUICK_EQUILIBRE) {
        count = 3; selection = 7u;
        labels[0] = "Partitions equilibrees (pivot milieu)";
        labels[1] = "Tableau aleatoire (pivot aleatoire)";
        labels[2] = "Toutes les valeurs egales (pivot aleatoire)";
        snprintf(data, sizeof data, "cases/quick_sort_best_avg_worst_cases.dat");
        snprintf(relative, sizeof relative, "cases/quick_sort_best_avg_worst_cases");
        snprintf(title, sizeof title, "Tri rapide - Partitions equilibrees, aleatoire et valeurs egales");
    } else {
        csv = 1; count = NOMBRE_ALGORITHMES;
        for (int i = 0; i < count; ++i) labels[i] = nom_algorithme((Algorithme)i);
        snprintf(data, sizeof data, "%s.csv", fichier_configuration((Configuration)c));
        snprintf(title, sizeof title, "Comparaison - %s%s", nom_configuration((Configuration)c),
                 small ? " - tailles jusqu a 5000" : "");
        if (selection == (1u << NOMBRE_ALGORITHMES) - 1u) {
            if (c == ALEATOIRE && !small) {
                csv = 0;
                snprintf(data, sizeof data, "benchmark/res.dat");
                snprintf(relative, sizeof relative, "algorithm_benchmark/algorithm_benchmark");
                snprintf(title, sizeof title, "Sorting Algorithm Performance");
            } else snprintf(relative, sizeof relative, "configurations/%s%s",
                            fichier_configuration((Configuration)c), small ? "_petites_tailles" : "");
        } else snprintf(relative, sizeof relative, "comparaisons/%s_algos_%02x%s",
                        fichier_configuration((Configuration)c), selection, small ? "_petites_tailles" : "");
    }
    return montrer_courbe(d, data, relative, title, csv, count, labels, selection, small);
}

int afficher_courbes(const DossierExperience *d, int (*read_int)(const char *, int, int))
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
            montrer_selection(d, QUICK_EQUILIBRE, -1, 1u << RAPIDE, 0);
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
        montrer_selection(d, c, a, selection, small);
    }
}

int generer_histogramme(const DossierExperience *d, Configuration configuration, int n, int open_image)
{
    LigneResultat rows[NOMBRE_CAS * NOMBRE_TAILLES];
    int total;
    if (!charger_resume(d, rows, &total)) return 0;
    if (configuration < ALEATOIRE || configuration >= NOMBRE_CAS) return 0;
    int special = configuration >= QUICK_EQUILIBRE;
    int count = special ? 3 : NOMBRE_ALGORITHMES;
    const char *labels[NOMBRE_ALGORITHMES];
    const char *quick[] = {"Partitions equilibrees (pivot milieu)",
                          "Entree aleatoire (pivot aleatoire)", "Toutes egales (pivot aleatoire)"};
    int measured[NOMBRE_ALGORITHMES] = {0};
    double minimum[NOMBRE_ALGORITHMES] = {0}, average[NOMBRE_ALGORITHMES] = {0};
    for (int i = 0; i < count; ++i) {
        const LigneResultat *r = trouver(rows, total, n, special ? (Configuration)(QUICK_EQUILIBRE + i) : configuration);
        if (!r) { puts("Configuration ou taille absente du resume."); return 0; }
        int a = special ? RAPIDE : i;
        labels[i] = special ? quick[i] : nom_algorithme((Algorithme)a);
        measured[i] = r->resultat.effectue[a];
        minimum[i] = r->resultat.minimum[a]; average[i] = r->resultat.moyenne[a];
    }
    char folder[LONGUEUR_CHEMIN], base[128], title[256];
    char scripts[2][LONGUEUR_CHEMIN], png[2][LONGUEUR_CHEMIN];
    if (!assurer_dossier(d->graphiques) ||
        !chemin(folder, sizeof folder, "%s/histogrammes", d->graphiques) || !assurer_dossier(folder)) return 0;
    snprintf(base, sizeof base, "%s_%d", special ? "experience_rapide" : fichier_configuration(configuration), n);
    snprintf(title, sizeof title, "%s - N=%d - run %s",
             special ? "Experience tri rapide : strategies de pivot indiquees" : nom_configuration(configuration), n, d->identifiant);
    if (!ecrire_histogramme(d, base, title, count, labels, measured, minimum, average, scripts[0], png[0])) return 0;
    int views = 1;
    double slow = 0, fast = 0;
    if (!special) {
        for (int i = 0; i < count; ++i) if (measured[i]) {
            if (i < RAPIDE && average[i] > slow) slow = average[i];
            if (i >= RAPIDE && average[i] > fast) fast = average[i];
        }
        if (fast > 0 && slow > 5 * fast) {
            size_t used = strlen(base);
            snprintf(base + used, sizeof base - used, "_rapides");
            size_t length = strlen(title);
            snprintf(title + length, sizeof title - length, " - rapide, fusion et tas");
            if (!ecrire_histogramme(d, base, title, 3, labels + RAPIDE, measured + RAPIDE,
                                   minimum + RAPIDE, average + RAPIDE, scripts[1], png[1])) return 0;
            views = 2;
        }
    }
    char exe[LONGUEUR_CHEMIN];
    int ok = trouver_gnuplot(exe, sizeof exe);
    if (!ok) puts("Gnuplot introuvable : donnees et scripts conserves. Utiliser ensuite l'option 5.");
    for (int i = 0; i < views; ++i) {
        if (ok && !executer_gnuplot(exe, scripts[i])) {
            fprintf(stderr, "Echec de Gnuplot : %s\n", scripts[i]); ok = 0;
        }
        printf("Histogramme : %s\nScript : %s\n", png[i], scripts[i]);
    }
    /* Print actual data paths (same basename as each script). */
    for (int i = 0; i < views; ++i) {
        char data[LONGUEUR_CHEMIN];
        snprintf(data, sizeof data, "%s", scripts[i]);
        char *suffix = strrchr(data, '.');
        if (suffix) strcpy(suffix, ".dat");
        printf("Fichier de donnees : %s\n", data);
    }
    if (ok && open_image && !ouvrir_png(png[0]))
        printf("Visionneuse indisponible. Ouvrir manuellement : %s\n", png[0]);
    return ok;
}

int afficher_histogramme(const DossierExperience *d, int (*read_int)(const char *, int, int))
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
