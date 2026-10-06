/* Compile the same sorting kernels twice: plain timing and untimed counting.
   The plain kernels contain no counter writes or counter-mode branches. */
#ifdef TRI_KERNEL_PASS

static void K(swap)(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
    ADD_MOVES(3);
}

static void K(insertionSort)(int *arr, int n)
{
    for (int i = 1; i < n; ++i) {
        int key = arr[i], j = i - 1;
        while (j >= 0 && GT(arr[j], key)) {
            arr[j + 1] = arr[j]; ADD_MOVES(1);
            --j;
        }
        arr[j + 1] = key; ADD_MOVES(1);
    }
}

static void K(selectionSort)(int *arr, int n)
{
    for (int i = 0; i < n - 1; ++i) {
        int min_idx = i;
        for (int j = i + 1; j < n; ++j)
            if (LT(arr[j], arr[min_idx])) min_idx = j;
        if (min_idx != i) K(swap)(&arr[i], &arr[min_idx]);
    }
}

static void K(bubbleSort)(int *arr, int n)
{
    for (int i = 0; i < n - 1; ++i) {
        int swapped = 0;
        for (int j = 0; j < n - i - 1; ++j) {
            if (GT(arr[j], arr[j + 1])) {
                K(swap)(&arr[j], &arr[j + 1]);
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

static int K(partition)(int *arr, int low, int high)
{
    int random_pivot = tirer_pivot(low, high);
    K(swap)(&arr[random_pivot], &arr[high]);
    int pivot = arr[high], i = low - 1;
    for (int j = low; j < high; ++j)
        if (LT(arr[j], pivot)) K(swap)(&arr[++i], &arr[j]);
    K(swap)(&arr[i + 1], &arr[high]);
    return i + 1;
}

static int K(partitionMid)(int *arr, int low, int high)
{
    int mid = low + (high - low) / 2;
    K(swap)(&arr[mid], &arr[high]);
    int pivot = arr[high], i = low - 1;
    for (int j = low; j < high; ++j)
        if (LT(arr[j], pivot)) K(swap)(&arr[++i], &arr[j]);
    K(swap)(&arr[i + 1], &arr[high]);
    return i + 1;
}

static void K(quickSortRec)(int *arr, int low, int high)
{
    while (low < high) {
        int pi = K(partition)(arr, low, high);
        if (pi - low < high - pi) {
            K(quickSortRec)(arr, low, pi - 1);
            low = pi + 1;
        } else {
            K(quickSortRec)(arr, pi + 1, high);
            high = pi - 1;
        }
    }
}

static void K(quickSortMidRec)(int *arr, int low, int high)
{
    while (low < high) {
        int pi = K(partitionMid)(arr, low, high);
        if (pi - low < high - pi) {
            K(quickSortMidRec)(arr, low, pi - 1);
            low = pi + 1;
        } else {
            K(quickSortMidRec)(arr, pi + 1, high);
            high = pi - 1;
        }
    }
}

static void K(mergeSortRecursive)(int *arr, int *temp, int left, int right)
{
    if (left >= right) return;
    int middle = left + (right - left) / 2;
    K(mergeSortRecursive)(arr, temp, left, middle);
    K(mergeSortRecursive)(arr, temp, middle + 1, right);
    int i = left, j = middle + 1, k = left;
    while (i <= middle && j <= right) {
        temp[k++] = LE(arr[i], arr[j]) ? arr[i++] : arr[j++]; ADD_MOVES(1);
    }
    while (i <= middle) { temp[k++] = arr[i++]; ADD_MOVES(1); }
    while (j <= right) { temp[k++] = arr[j++]; ADD_MOVES(1); }
    for (i = left; i <= right; ++i) { arr[i] = temp[i]; ADD_MOVES(1); }
}

static void K(heapify)(int *arr, int n, int i)
{
    while (i < n / 2) {
        int child = 2 * i + 1;
        if (child + 1 < n && GT(arr[child + 1], arr[child])) ++child;
        if (!GT(arr[child], arr[i])) break;
        K(swap)(&arr[i], &arr[child]);
        i = child;
    }
}

static void K(heapSort)(int *arr, int n)
{
    for (int i = n / 2 - 1; i >= 0; --i) K(heapify)(arr, n, i);
    for (int i = n - 1; i > 0; --i) {
        K(swap)(&arr[0], &arr[i]);
        K(heapify)(arr, i, 0);
    }
}

static void K(dispatch)(Algorithme algorithme, int *arr, int *temp, int n, int mid)
{
    switch (algorithme) {
    case SELECTION: K(selectionSort)(arr, n); break;
    case BULLES: K(bubbleSort)(arr, n); break;
    case INSERTION: K(insertionSort)(arr, n); break;
    case RAPIDE:
        if (mid) K(quickSortMidRec)(arr, 0, n - 1);
        else K(quickSortRec)(arr, 0, n - 1);
        break;
    case FUSION: K(mergeSortRecursive)(arr, temp, 0, n - 1); break;
    case TAS: K(heapSort)(arr, n); break;
    }
}

#else
#include "tri.h"
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static uint32_t etat_pivots = 1;
static int tirer_pivot(int low, int high);

#define TRI_KERNEL_PASS
#define K(name) plain_##name
#define LT(a,b) ((a) < (b))
#define GT(a,b) ((a) > (b))
#define LE(a,b) ((a) <= (b))
#define ADD_MOVES(n) ((void)0)
#include "tri.c"
#undef K
#undef LT
#undef GT
#undef LE
#undef ADD_MOVES

static Operations compteur;
#define K(name) counted_##name
#define LT(a,b) (++compteur.comparaisons, (a) < (b))
#define GT(a,b) (++compteur.comparaisons, (a) > (b))
#define LE(a,b) (++compteur.comparaisons, (a) <= (b))
#define ADD_MOVES(n) (compteur.mouvements += (n))
#include "tri.c"
#undef K
#undef LT
#undef GT
#undef LE
#undef ADD_MOVES
#undef TRI_KERNEL_PASS

const int TAILLES[NOMBRE_TAILLES] = {
    100, 500, 1000, 5000, 10000, 20000, 50000, 100000, 500000, 1000000
};
static const char *const NOMS_ALGORITHMES[] = {
    "Tri par selection", "Tri a bulles", "Tri par insertion",
    "Tri rapide", "Tri fusion", "Tri par tas"
};
static const char *const FICHIERS_ALGORITHMES[] = {
    "selection_sort", "bubble_sort", "insertion_sort", "quick_sort_input_order",
    "merge_sort", "heap_sort"
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
const char *nom_algorithme(Algorithme a) { return NOMS_ALGORITHMES[a]; }
const char *fichier_algorithme(Algorithme a) { return FICHIERS_ALGORITHMES[a]; }
const char *nom_configuration(Configuration c) { return NOMS_CAS[c]; }
const char *fichier_configuration(Configuration c) { return FICHIERS_CAS[c]; }

void appliquer_tri(Algorithme a, int *arr, int *temp, int n, int mid)
{ plain_dispatch(a, arr, temp, n, mid); }
Operations compter_tri(Algorithme a, int *arr, int *temp, int n, int mid)
{
    compteur = (Operations){0, 0};
    counted_dispatch(a, arr, temp, n, mid);
    return compteur;
}

uint32_t nombre_aleatoire(uint32_t *etat)
{
    uint32_t x = *etat;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    *etat = x;
    return x;
}
void initialiser_pivots(uint32_t graine)
{
    etat_pivots = graine ? graine : 1u;
}
/* xorshift32 has 2^32-1 nonzero outputs. Subtract one to get [0,2^32-2],
   then accept only a multiple of the partition width before taking modulo. */
static int tirer_pivot(int low, int high)
{
    uint32_t largeur = (uint32_t)(high - low + 1);
    uint32_t limite = UINT32_MAX - UINT32_MAX % largeur;
    uint32_t valeur;
    do { valeur = nombre_aleatoire(&etat_pivots) - 1u; } while (valeur >= limite);
    return low + (int)(valeur % largeur);
}
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
int tableau_est_trie(const int *arr, int n)
{
    for (int i = 1; i < n; ++i) if (arr[i - 1] > arr[i]) return 0;
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
                       FILE *mesures, ResultatExperience *result, int progress)
{
    LARGE_INTEGER frequency, start, finish;
    int *original = NULL, *copy = NULL, *scratch = NULL;
    int ok = 0;
    memset(result, 0, sizeof *result);
    if (n < 1 || n > TAILLE_MAXIMALE || p->repetitions < 1 || p->repetitions > 100 ||
        p->limite_lente < 1 || c < ALEATOIRE || c >= NOMBRE_CAS || !seed || !mesures ||
        !QueryPerformanceFrequency(&frequency) || frequency.QuadPart <= 0) {
        fputs("Parametres ou chronometre invalides.\n", stderr); return 0;
    }
    int eligibles = 0;
    for (int a = 0; a < NOMBRE_ALGORITHMES; ++a) {
        result->effectue[a] = algorithme_eligible((Algorithme)a, c, n, p->limite_lente);
        eligibles += result->effectue[a];
        result->minimum[a] = DBL_MAX;
        if (progress && !result->effectue[a] && c < QUICK_EQUILIBRE)
            printf("  %s : non_mesure (limite %d)\n", nom_algorithme((Algorithme)a), p->limite_lente);
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
            if (progress) {
                printf("  %s | %s | N=%d | repetition %d/%d\n",
                       nom_algorithme((Algorithme)a), nom_configuration(c), n, r + 1, p->repetitions);
                fflush(stdout);
            }
            memcpy(copy, original, (size_t)n * sizeof *copy);
            initialiser_pivots(pivot_seed);
            if (!QueryPerformanceCounter(&start)) goto timer_error;
            appliquer_tri((Algorithme)a, copy, scratch, n, c == QUICK_EQUILIBRE);
            if (!QueryPerformanceCounter(&finish)) goto timer_error;
            if (!tableau_est_trie(copy, n)) goto sort_error;
            double seconds = (double)(finish.QuadPart - start.QuadPart) / (double)frequency.QuadPart;
            if (seconds < 0) goto timer_error;
            if (seconds < result->minimum[a]) result->minimum[a] = seconds;
            result->moyenne[a] += seconds / p->repetitions;
            if (fprintf(mesures, "%s;%d;%d;%s;%u;%u;%.12f\n", fichier_configuration(c), n,
                        r + 1, fichier_algorithme((Algorithme)a), (unsigned)input_seed,
                        (unsigned)pivot_seed, seconds) < 0) goto file_error;
            if (r == 0) {
                if (progress) { puts("    Comptage separe (hors chronometrage)..."); fflush(stdout); }
                memcpy(copy, original, (size_t)n * sizeof *copy);
                initialiser_pivots(pivot_seed);
                result->operations[a] = compter_tri((Algorithme)a, copy, scratch, n, c == QUICK_EQUILIBRE);
                if (!tableau_est_trie(copy, n)) goto sort_error;
            }
        }
    }
    if (fflush(mesures) != 0) goto file_error;
    ok = 1;
    goto fin;
timer_error: fputs("Echec du chronometre.\n", stderr); goto fin;
sort_error: fputs("Echec de verification du tri.\n", stderr); goto fin;
file_error: fputs("Echec d'ecriture des mesures.\n", stderr);
fin:
    free(original); free(copy); free(scratch);
    return ok;
}
#endif
