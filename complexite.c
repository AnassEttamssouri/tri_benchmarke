#include "complexite.h"
#include <math.h>
#include <string.h>

const char *nom_modele(ModeleCroissance modele)
{
    const char *names[] = {"n", "n log n", "n^2", "non_evalue"};
    return names[modele];
}
const char *nom_verification(VerificationExacte verification)
{
    const char *names[] = {"non_applicable", "verifie", "ecart_exact"};
    return names[verification];
}
const char *const *descriptions_theoriques(int *nombre)
{
    static const char *const lines[] = {
        "\nComplexites theoriques (meilleur / moyen / pire) :",
        "Bulles avec arret anticipe : O(n) / O(n^2) / O(n^2)",
        "Selection                 : O(n^2) / O(n^2) / O(n^2)",
        "Insertion                 : O(n) / O(n^2) / O(n^2)",
        "Rapide, pivot aleatoire   : O(n log n) / O(n log n) / O(n^2)",
        "Fusion                    : Theta(n log n) dans les trois cas ici.",
        "Tas                       : O(n log n); cas moyen/pire Theta(n log n).",
        "Tas : toutes valeurs egales donnent Theta(n) avec cette implementation.",
        "Partition rapide en deux groupes : les doublons peuvent provoquer O(n^2).",
        "Valeurs theoriques; les mesures en secondes et les compteurs sont reels.",
        "Minimum chronometre != meilleur cas theorique d'entree.",
        "Compteurs : un passage representatif, pas une moyenne; un swap = 3 mouvements.",
        "Reference : Sedgewick et Wayne, Algorithms, 4e edition :",
        "https://algs4.cs.princeton.edu/cheatsheet/",
        "https://algs4.cs.princeton.edu/23quicksort/ (hypotheses et partition differentes)."
    };
    *nombre = (int)(sizeof lines / sizeof lines[0]);
    return lines;
}
const char *reference_complexite(Algorithme a, Configuration c)
{
    if (a == SELECTION) return "Theta(n^2) pour toute entree";
    if (a == BULLES || a == INSERTION) {
        if (c == CROISSANT) return "Theta(n) sur entree croissante";
        if (c == DECROISSANT) return "Theta(n^2) sur entree inverse";
        if (c == PRESQUE_TRIE && a == INSERTION) return "O(n+I), I = nombre d'inversions";
        return "O(n^2), cout dependant de l'entree";
    }
    if (a == RAPIDE) {
        if (c == QUICK_EQUILIBRE) return "Theta(n log n), partitions construites equilibrees";
        if (c == QUICK_EGAUX) return "Theta(n^2), partition en deux groupes";
        if (c == VALEURS_REPETEES) return "O(n^2), sensible aux doublons";
        return "n log n attendu sous hypotheses, pire O(n^2)";
    }
    if (a == FUSION) return "Theta(n log n), fusion systematique";
    return "O(n log n), Theta(n) possible sur valeurs toutes egales";
}

EstimationCroissance estimer_croissance(const double *tailles, const double *valeurs, int nombre)
{
    EstimationCroissance result = {0, MODELE_INDETERMINE, 0, 0};
    double x[NOMBRE_TAILLES], y[NOMBRE_TAILLES];
    if (!tailles || !valeurs || nombre < 0 || nombre > NOMBRE_TAILLES) return result;
    for (int i = 0; i < nombre; ++i) {
        if (!isfinite(tailles[i]) || !isfinite(valeurs[i]) || tailles[i] < 2 || valeurs[i] <= 0) continue;
        x[result.points] = log(tailles[i]);
        y[result.points++] = log(valeurs[i]);
    }
    if (result.points < 3) return result;
    double mean_x = 0, mean_y = 0;
    for (int i = 0; i < result.points; ++i) { mean_x += x[i]; mean_y += y[i]; }
    mean_x /= result.points; mean_y /= result.points;
    double xx = 0, xy = 0;
    for (int i = 0; i < result.points; ++i) {
        xx += (x[i] - mean_x) * (x[i] - mean_x);
        xy += (x[i] - mean_x) * (y[i] - mean_y);
    }
    if (!(xx > 0)) return result;
    result.exposant = xy / xx;
    result.dispersion = HUGE_VAL;
    for (int m = MODELE_LINEAIRE; m <= MODELE_QUADRATIQUE; ++m) {
        double ratios[NOMBRE_TAILLES], mean = 0, variance = 0;
        for (int i = 0; i < result.points; ++i) {
            double model_log = m == MODELE_LINEAIRE ? x[i] :
                               m == MODELE_NLOGN ? x[i] + log(x[i]) : 2 * x[i];
            ratios[i] = y[i] - model_log;
            mean += ratios[i];
        }
        mean /= result.points;
        for (int i = 0; i < result.points; ++i) variance += (ratios[i] - mean) * (ratios[i] - mean);
        double dispersion = sqrt(variance / result.points);
        if (dispersion < result.dispersion) {
            result.dispersion = dispersion;
            result.modele = (ModeleCroissance)m;
        }
    }
    if (result.dispersion > DISPERSION_MAXIMALE) result.modele = MODELE_INDETERMINE;
    return result;
}

static uint64_t comparaisons_equilibrees(int n)
{
    if (n < 2) return 0;
    int left = (n - 1) / 2;
    return (uint64_t)n - 1u + comparaisons_equilibrees(left) + comparaisons_equilibrees(n - 1 - left);
}
static int comparaisons_exactes(Algorithme a, Configuration c, int n, uint64_t *attendu)
{
    if (a == SELECTION || (a == RAPIDE && c == QUICK_EGAUX))
        *attendu = (uint64_t)n * (uint64_t)(n - 1) / 2u;
    else if ((a == BULLES || a == INSERTION) && c == CROISSANT)
        *attendu = (uint64_t)n - 1u;
    else if (a == RAPIDE && c == QUICK_EQUILIBRE)
        *attendu = comparaisons_equilibrees(n);
    else return 0;
    return 1;
}
int analyser_complexites(const LigneResultat *rows, int count, AnalyseComplexite *analyses, int capacity)
{
    if (!rows || !analyses || count < 0 || count > NOMBRE_CAS * NOMBRE_TAILLES) return 0;
    int total = 0;
    for (int c = 0; c < NOMBRE_CAS; ++c) {
        if (!configuration_presente(rows, count, (Configuration)c)) continue;
        for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
            if (c >= QUICK_EQUILIBRE && a != RAPIDE) continue;
            if (total >= capacity) return total;
            AnalyseComplexite *analysis = &analyses[total++];
            memset(analysis, 0, sizeof *analysis);
            analysis->configuration = (Configuration)c; analysis->algorithme = (Algorithme)a;
            analysis->reference = reference_complexite((Algorithme)a, (Configuration)c);
            double sizes_ops[NOMBRE_TAILLES], ops[NOMBRE_TAILLES];
            double sizes_times[NOMBRE_TAILLES], times[NOMBRE_TAILLES];
            int n_ops = 0, n_times = 0;
            for (int s = 0; s < NOMBRE_TAILLES; ++s) {
                const LigneResultat *row = trouver(rows, count, TAILLES[s], (Configuration)c);
                if (!row || !row->resultat.effectue[a]) continue;
                const Operations *op = &row->resultat.operations[a];
                double operations = (double)op->comparaisons + (double)op->mouvements;
                if (operations > 0) { sizes_ops[n_ops] = row->taille; ops[n_ops++] = operations; }
                if (row->taille >= 1000 && row->resultat.minimum[a] > 0) {
                    sizes_times[n_times] = row->taille; times[n_times++] = row->resultat.minimum[a];
                }
                uint64_t attendu;
                if (comparaisons_exactes((Algorithme)a, (Configuration)c, row->taille, &attendu)) {
                    ++analysis->points_exacts;
                    if (op->comparaisons != attendu) ++analysis->ecarts_exacts;
                }
            }
            analysis->operations = estimer_croissance(sizes_ops, ops, n_ops);
            analysis->temps = estimer_croissance(sizes_times, times, n_times);
            analysis->verification = !analysis->points_exacts ? EXACT_NON_APPLICABLE :
                                     analysis->ecarts_exacts ? EXACT_ECART : EXACT_VERIFIE;
        }
    }
    return total;
}
