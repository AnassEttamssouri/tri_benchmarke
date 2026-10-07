#include "benchmark.h"
#include "systeme.h"
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const int TAILLES[NOMBRE_TAILLES] = {
    100, 500, 1000, 5000, 10000, 20000, 50000, 100000, 500000, 1000000
};
static const char *const NOMS_CAS[] = {
    "Tableau aleatoire", "Tableau deja trie", "Tableau inverse",
    "Tableau presque trie", "Dix valeurs repetees", "Partitions equilibrees",
    "Quick aleatoire", "Toutes les valeurs egales"
};
static const char *const FICHIERS_CAS[] = {
    "aleatoire", "croissant", "decroissant", "presque_trie", "valeurs_repetees",
    "quick_equilibre", "quick_aleatoire", "quick_egaux"
};
const char *nom_configuration(Configuration c) { return NOMS_CAS[c]; }
const char *fichier_configuration(Configuration c) { return FICHIERS_CAS[c]; }

uint32_t nombre_aleatoire(uint32_t *etat)
{
    uint32_t x = *etat;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    *etat = x;
    return x;
}
uint32_t graine_campagne(void) { uint32_t seed = (uint32_t)time(NULL); return seed ? seed : 1u; }

uint32_t graine_experience(uint32_t seed, Configuration c, int n, int r, int pivot)
{
    uint32_t x = seed ^ ((uint32_t)c + 1u) * 0x9e3779b9u ^
                 (uint32_t)n * 0x85ebca6bu ^ ((uint32_t)r + 1u) * 0xc2b2ae35u;
    if (pivot) x ^= 0x27d4eb2fu;
    if (!x) x = 1;
    return nombre_aleatoire(&x);
}

/* Original balanced-partition construction; allocation failures propagate. */
static int buildBest(int *out, int n, int lo)
{
    if (n <= 0) return 1;
    if (n == 1) { out[0] = lo; return 1; }
    int a = (n - 1) / 2, b = n - 1 - a, p = lo + a;
    int *l = a ? malloc((size_t)a * sizeof *l) : NULL;
    int *right = malloc((size_t)b * sizeof *right);
    if ((a && !l) || !right || !buildBest(l, a, lo) || !buildBest(right, b, p + 1)) {
        free(l); free(right); return 0;
    }
    for (int i = 0; i < a; ++i) out[i] = l[i];
    out[a] = right[b - 1];
    for (int i = 0; i < b - 1; ++i) out[a + 1 + i] = right[i];
    out[n - 1] = p;
    int mid = (n - 1) / 2, t = out[mid];
    out[mid] = out[n - 1]; out[n - 1] = t;
    free(l); free(right); return 1;
}

int generer_tableau(int *arr, int n, Configuration c, uint32_t seed)
{
    if (n < 0 || n > TAILLE_MAXIMALE || c < ALEATOIRE || c >= NOMBRE_CAS || !seed)
        return 0;
    if (c == QUICK_EQUILIBRE) return buildBest(arr, n, 0);
    for (int i = 0; i < n; ++i) {
        switch (c) {
        case ALEATOIRE: case QUICK_ALEATOIRE:
            arr[i] = (int)(nombre_aleatoire(&seed) % 1000001u); break;
        case CROISSANT: case PRESQUE_TRIE: arr[i] = i; break;
        case DECROISSANT: arr[i] = n - i - 1; break;
        case VALEURS_REPETEES: arr[i] = (int)(nombre_aleatoire(&seed) % 10u); break;
        case QUICK_EGAUX: arr[i] = 42; break;
        default: return 0;
        }
    }
    if (c == PRESQUE_TRIE && n > 1) {
        int exchanges = n / 100;
        if (!exchanges) exchanges = 1;
        for (int i = 0; i < exchanges; ++i) {
            int a = (int)(nombre_aleatoire(&seed) % (uint32_t)n);
            int b = (a + 1 + (int)(nombre_aleatoire(&seed) % (uint32_t)(n - 1))) % n;
            int t = arr[a]; arr[a] = arr[b]; arr[b] = t;
        }
    }
    return 1;
}
int algorithme_eligible(Algorithme a, Configuration c, int n, int limit)
{
    if (c >= QUICK_EQUILIBRE && a != RAPIDE) return 0;
    if (n > limit && (a <= INSERTION ||
        (a == RAPIDE && (c == VALEURS_REPETEES || c == QUICK_EGAUX)))) return 0;
    return 1;
}

int executer_experience(int n, Configuration c, const Parametres *p, uint32_t seed,
                       EnregistrerMesure enregistrer, void *contexte, ResultatExperience *result, ProgressionBenchmark progression)
{
    int64_t start, finish;
    int *original = NULL, *copy = NULL, *scratch = NULL;
    int ok = 0;
    if (!p || !result) return 0;
    memset(result, 0, sizeof *result);
    if (n < 1 || n > TAILLE_MAXIMALE || p->repetitions < 1 || p->repetitions > 100 ||
        p->limite_lente < 1 || c < ALEATOIRE || c >= NOMBRE_CAS || !seed || !enregistrer ||
        !chrono_disponible()) {
        fputs("Parametres ou chronometre invalides.\n", stderr); return 0;
    }
    int eligibles = 0;
    for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
        result->effectue[a] = algorithme_eligible((Algorithme)a, c, n, p->limite_lente);
        eligibles += result->effectue[a];
        result->minimum[a] = DBL_MAX;
        if (progression && !result->effectue[a] && c < QUICK_EQUILIBRE)
            progression(PROGRESSION_EXCLU, (Algorithme)a, c, n, 0, p);
    }
    result->graine_entree = graine_experience(seed, c, n, 0, 0);
    result->graine_pivot = graine_experience(seed, c, n, 0, 1);
    if (!eligibles) return 1; /* No generation, allocation or repetitions. */
    original = malloc((size_t)n * sizeof *original);
    copy = malloc((size_t)n * sizeof *copy);
    scratch = malloc((size_t)n * sizeof *scratch);
    if (!original || !copy || !scratch) {
        fputs("Echec d'allocation des tableaux.\n", stderr); goto fin;
    }
    for (int r = 0; r < p->repetitions; ++r) {
        uint32_t input_seed = graine_experience(seed, c, n, r, 0);
        uint32_t pivot_seed = graine_experience(seed, c, n, r, 1);
        if (!generer_tableau(original, n, c, input_seed)) {
            fputs("Echec de generation du tableau.\n", stderr); goto fin;
        }
        if (r == 0) { result->graine_entree = input_seed; result->graine_pivot = pivot_seed; }
        for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
            if (!result->effectue[a]) continue;
            if (progression) progression(PROGRESSION_MESURE, (Algorithme)a, c, n, r + 1, p);
            memcpy(copy, original, (size_t)n * sizeof *copy);
            initialiser_pivots(pivot_seed);
            if (!chrono_instant(&start)) goto timer_error;
            appliquer_tri((Algorithme)a, copy, scratch, n, c == QUICK_EQUILIBRE);
            if (!chrono_instant(&finish)) goto timer_error;
            if (!tableau_est_trie(copy, n)) goto sort_error;
            double seconds = chrono_secondes(start, finish);
            if (seconds < 0) goto timer_error;
            if (seconds < result->minimum[a]) result->minimum[a] = seconds;
            result->moyenne[a] += seconds / p->repetitions;
            MesureBenchmark mesure = {c, (Algorithme)a, n, r + 1, input_seed, pivot_seed, seconds};
            if (!enregistrer(&mesure, contexte)) goto file_error;
            if (r == 0) {
                if (progression) progression(PROGRESSION_COMPTE, (Algorithme)a, c, n, 1, p);
                memcpy(copy, original, (size_t)n * sizeof *copy);
                initialiser_pivots(pivot_seed);
                result->operations[a] = compter_tri((Algorithme)a, copy, scratch, n, c == QUICK_EQUILIBRE);
                if (!tableau_est_trie(copy, n)) goto sort_error;
            }
        }
    }
    if (!enregistrer(NULL, contexte)) goto file_error;
    ok = 1;
    goto fin;
timer_error: fputs("Echec du chronometre.\n", stderr); goto fin;
sort_error: fputs("Echec de verification du tri.\n", stderr); goto fin;
file_error: fputs("Echec d'ecriture des mesures.\n", stderr);
fin:
    free(original); free(copy); free(scratch);
    return ok;
}

const LigneResultat *trouver(const LigneResultat *rows, int count, int n, Configuration c)
{
    for (int i = 0; i < count; ++i)
        if (rows[i].taille == n && rows[i].configuration == c) return &rows[i];
    return NULL;
}
int configuration_presente(const LigneResultat *rows, int count, Configuration c)
{
    for (int i = 0; i < count; ++i) if (rows[i].configuration == c) return 1;
    return 0;
}
int executer_campagne(const Parametres *p, int choice, uint32_t seed,
                     EnregistrerMesure enregistrer, void *contexte, LigneResultat *rows,
                     int *count, ProgressionBenchmark progression,
                     void (*rapporter)(const LigneResultat *))
{
    if (!p || !rows || !count || choice < -1 || choice > NOMBRE_CONFIGURATIONS) return 0;
    *count = 0;
    for (int c = 0; c < NOMBRE_CAS; ++c) {
        if (choice >= 0 && ((choice < NOMBRE_CONFIGURATIONS && c != choice) ||
            (choice == NOMBRE_CONFIGURATIONS && c < QUICK_EQUILIBRE))) continue;
        for (int s = 0; s < NOMBRE_TAILLES && TAILLES[s] <= p->taille_max; ++s) {
            LigneResultat *row = &rows[*count];
            row->taille = TAILLES[s]; row->configuration = (Configuration)c;
            if (!executer_experience(row->taille, row->configuration, p, seed, enregistrer,
                                    contexte, &row->resultat, progression)) {
                fprintf(stderr, "Test interrompu : %s, N=%d.\n", nom_configuration((Configuration)c), row->taille);
                return 0;
            }
            ++*count;
            if (rapporter) rapporter(row);
        }
    }
    return 1;
}
