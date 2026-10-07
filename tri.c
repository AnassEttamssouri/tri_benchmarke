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

static const char *const NOMS_ALGORITHMES[] = {
    "Tri par selection", "Tri a bulles", "Tri par insertion",
    "Tri rapide", "Tri fusion", "Tri par tas"
};
static const char *const FICHIERS_ALGORITHMES[] = {
    "selection_sort", "bubble_sort", "insertion_sort", "quick_sort_input_order",
    "merge_sort", "heap_sort"
};
const char *nom_algorithme(Algorithme a) { return NOMS_ALGORITHMES[a]; }
const char *fichier_algorithme(Algorithme a) { return FICHIERS_ALGORITHMES[a]; }

void appliquer_tri(Algorithme a, int *arr, int *temp, int n, int mid)
{ plain_dispatch(a, arr, temp, n, mid); }
Operations compter_tri(Algorithme a, int *arr, int *temp, int n, int mid)
{
    compteur = (Operations){0, 0};
    counted_dispatch(a, arr, temp, n, mid);
    return compteur;
}

static uint32_t prochain_pivot(uint32_t *etat)
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
    do { valeur = prochain_pivot(&etat_pivots) - 1u; } while (valeur >= limite);
    return low + (int)(valeur % largeur);
}
int tableau_est_trie(const int *arr, int n)
{
    for (int i = 1; i < n; ++i) if (arr[i - 1] > arr[i]) return 0;
    return 1;
}
#endif
