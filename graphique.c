#include "graphique.h"
#include <string.h>
#include <stdlib.h>

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

static int regenerer_script(const char *script, void *exe)
{
    if (executer_gnuplot((const char *)exe, script)) return 1;
    fprintf(stderr, "Echec de regeneration : %s\n", script);
    return 0;
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
        char directory[LONGUEUR_CHEMIN];
        if (!chemin(directory, sizeof directory, "%s/%s", d->graphiques, folders[folder])) return 0;
        if (!parcourir_fichiers(directory, "*.plt", regenerer_script, exe, &count)) ok = 0;
    }
    if (!count) { puts("Aucune donnee disponible pour les graphiques."); return 0; }
    if (ok) printf("%d graphiques PNG crees dans %s.\n", count, d->graphiques);
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

int afficher_courbe(const DossierExperience *d, const DemandeCourbe *demande)
{
    if (!d || !demande) return 0;
    if (demande->type == COURBE_CONFIGURATIONS) {
        if (demande->algorithme < SELECTION || demande->algorithme >= NOMBRE_ALGORITHMES) return 0;
        return montrer_selection(d, NOMBRE_CONFIGURATIONS, demande->algorithme, 0, 0);
    }
    if (demande->type == COURBE_RAPIDE) return montrer_selection(d, QUICK_EQUILIBRE, -1, 0, 0);
    if (demande->type != COURBE_COMPARAISON || demande->configuration < ALEATOIRE ||
        demande->configuration >= NOMBRE_CONFIGURATIONS || !demande->selection ||
        (demande->selection & ~((1u << NOMBRE_ALGORITHMES) - 1u))) return 0;
    return montrer_selection(d, demande->configuration, -1, demande->selection, demande->petites_tailles != 0);
}
