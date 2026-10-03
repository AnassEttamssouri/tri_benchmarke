
#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>



double measure_time(
    sort_func algorithm,
    int* arr,
    int n
)
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER end;

    QueryPerformanceFrequency(&frequency);

    QueryPerformanceCounter(&start);

    algorithm(arr, n);

    QueryPerformanceCounter(&end);

    return (double)(end.QuadPart - start.QuadPart)
           / (double)frequency.QuadPart;
}


void generate_global_comparaison(char* filename,int* sizes,int count){
    FILE* f = fopen(filename,"w");

    if(f == NULL){
        return ; 
    }


    fprintf(f,"# Size Selection_Sort Bubble_Sort Insertion_Sort Quick_Sort Merge_Sort \n");


    for(int i = 0 ; i < count ; i++){

        int *arr;
        generateRandom(&arr,sizes[i]);

        double duration ; 
        int* temp = (int*) malloc(sizes[i] * sizeof(int));;
        double best;

        best = 1e9;

        /*
            noise makes the algorithm slower ,
            never faster  so the following solution is the standard solution for the problem (CPU used by many programs).
            
        */


        //Selection Sort :
        for(int r = 0 ; r < 7 ; r++){
            memcpy(temp,arr,sizes[i]*sizeof(int));
            duration = measure_time(selectionSort,temp,sizes[i]);
            if(duration < best) best = duration;
        }
        fprintf(f,"%d %f ",sizes[i],best);

        best = 1e9;

        //Bubble Sort :
        for(int r = 0 ; r < 7 ; r++){
            memcpy(temp,arr,sizes[i]*sizeof(int));
            duration = measure_time(bubbleSort,temp,sizes[i]);
            if(duration < best) best = duration;
        }

        fprintf(f,"%f ",best);
        best = 1e9;

        //Insertion Sort :
        for(int r = 0 ; r < 7 ; r++){ 
            memcpy(temp,arr,sizes[i]*sizeof(int));
            duration = measure_time(insertionSort,temp,sizes[i]);
            if(duration < best) best = duration;
        }

        fprintf(f,"%f ",best);
        best = 1e9;

        //Quick Sort :
        for(int r = 0 ; r < 7 ; r++){
            memcpy(temp,arr,sizes[i]*sizeof(int));
            duration = measure_time(quickSort,temp,sizes[i]);
            if(duration < best) best = duration;
        }

        fprintf(f,"%f ",best);
        best = 1e9;

        //Merge Sort :
        for(int r = 0 ; r < 7 ; r++ ){ 
            memcpy(temp,arr,sizes[i]*sizeof(int));
            duration = measure_time(mergeSort,temp,sizes[i]);
            if(duration < best) best = duration;
        }

        fprintf(f,"%f\n",best);
        

        free(temp);
        free(arr);
    }
    fclose(f);

}

void benchmark_algorithm_cases(
    const char *filename,
    sort_func algo,
    int *sizes,
    int count
)
{
    FILE *file = fopen(filename, "w");

    if (file == NULL)
    {
        perror("Erreur ouverture fichier");
        return;
    }

    fprintf(file, "# N Sorted Random Reversed\n");

    for (int i = 0; i < count; i++)
    {
        int n = sizes[i];

        int *arr = NULL,*temp = (int*) malloc(n*sizeof(int));

        double sorted_time;
        double random_time;
        double reversed_time;
        double best = 1e9;

        /* Cas simple : tableau déjà trié */
        generateSorted(&arr, n);
        for(int r = 0 ; r < 7 ; r++){
            memcpy(temp,arr,n*sizeof(int));
            sorted_time = measure_time(algo, temp, n);
            if(sorted_time < best) best = sorted_time;
        }
        sorted_time = best;
        best = 1e9;
        free(arr);

        /* Cas moyen : tableau aléatoire */
        generateRandom(&arr, n);
        for(int r = 0 ; r < 7 ; r++){
            memcpy(temp,arr,n*sizeof(int));
            random_time = measure_time(algo, temp, n);
            if(random_time < best) best = random_time;
        }
        random_time = best; 
        best = 1e9;
        free(arr);

        /* Cas difficile : tableau inversé */
        generateReversed(&arr, n);
        for(int r = 0 ; r < 7 ; r++){
            memcpy(temp,arr,n*sizeof(int));
            reversed_time = measure_time(algo, temp, n);
            if(reversed_time < best) best = reversed_time;
        }
        reversed_time = best;
        free(arr);

        fprintf(
            file,
            "%d %.9f %.9f %.9f\n",
            n,
            sorted_time,
            random_time,
            reversed_time
        );
        free(temp);

    }

    fclose(file);
}

void benchmark_algorithm_quick(
    const char *filename,
    int *sizes,
    int count
)
{
    FILE *file = fopen(filename, "w");

    if (file == NULL)
    {
        perror("Erreur ouverture fichier");
        return;
    }

    fprintf(file, "# N 'Perfect partitioned' Random 'fully duplicates values'\n");

    for (int i = 0; i < count; i++)
    {
        int n = sizes[i];

        int *arr = NULL,*temp = (int*) malloc(n * sizeof(int));

        double simple_case_time;
        double average_case_time;
        double bad_case_time;
        double best = 1e9;

        /* Cas simple : tableau diviser en parties de mêmes taille (on utilise l'algorithme avec le pivot est le centre) */
        generateQuickBest(&arr,n);

        for(int r = 0 ; r < 7 ; r++){
            memcpy(temp,arr,n*sizeof(int));
            simple_case_time = measure_time(quickSortMid,temp,n);
            if(simple_case_time < best) best = simple_case_time;
        }
        simple_case_time = best;
        best = 1e9;
        
        free(arr);

        /* Cas moyen : tableau aléatoire (on utilise l'algorithme où le pivot est choisit aléatoirement) */
        generateRandom(&arr, n);
        for(int r = 0 ; r < 7 ; r++){
            memcpy(temp,arr,n*sizeof(int));
            average_case_time = measure_time(quickSort, temp, n);
            if(average_case_time < best) best = average_case_time;
        }
        
        average_case_time = best;
        best=1e9;
        free(arr);

        /* Cas difficile : tableau inversé */
        generateAllEqual(&arr, n);
        for(int r = 0 ; r < 7 ; r++){
            memcpy(temp,arr,n*sizeof(int));
            bad_case_time = measure_time(quickSort, temp, n);
            if(bad_case_time < best) best = bad_case_time ;
        }
        bad_case_time = best;
        free(arr);

        fprintf(
            file,
            "%d %.9f %.9f %.9f\n",
            n,
            simple_case_time,
            average_case_time,
            bad_case_time
        );
        free(temp);
    }

    fclose(file);
}



int generate_sizes(int start_size  , int jump , int end_size, int ** sizes){
    if(start_size <= 0 || end_size <= start_size || jump <=  0){
        return 0 ; 
    }

    int count = ((end_size-start_size)/jump )+ 1;

    *sizes = (int*) malloc(count*sizeof(int));
    if(*sizes == NULL){
        return 0;
    }
    for(int i = 0 ; i < count ; i++){
        *((*sizes)+i) = start_size + i*jump;
    }
    
    return count;
}


void generateSorted(int **arr, int n){
    *arr = (int*) malloc(n*sizeof(int));
    for(int i = 0 ; i < n ; i++){
        (*arr)[i] = i;
    }
}

void generateReversed(int **arr , int n){
    *arr = (int*) malloc(n*sizeof(int));
    for(int i = 0 ; i < n ; i++){
        (*arr)[i] = n-i-1;
    }
}

void generateRandom(int** arr , int n){
    *arr = (int*) malloc(n*sizeof(int));
    for(int i = 0 ; i < n ; i++){
        (*arr)[i] = rand();
    }
}

void generateAllEqual(int **arr, int n) {
    *arr = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) (*arr)[i] = 42;
}

// Fills out[0..n-1] with values lo..lo+n-1, arranged so quickSortMid
// splits perfectly (pivot = median) at every level.
static void buildBest(int *out, int n, int lo) {
    if (n == 1) { out[0] = lo; return; }

    int a = (n - 1) / 2;          // elements smaller than pivot
    int b = n - 1 - a;            // elements larger than pivot
    int p = lo + a;               // pivot = median value

    int *L = malloc(a * sizeof(int));
    int *R = malloc(b * sizeof(int));
    if (a > 0) buildBest(L, a, lo);
    buildBest(R, b, p + 1);

    // Layout just before the final partition swap
    for (int i = 0; i < a; i++) out[i] = L[i];
    out[a] = R[b - 1];
    for (int i = 0; i < b - 1; i++) out[a + 1 + i] = R[i];
    out[n - 1] = p;

    // Undo the "pivot to the end" swap
    int mid = (n - 1) / 2;
    int t = out[mid]; out[mid] = out[n - 1]; out[n - 1] = t;

    free(L); free(R);
}

void generateQuickBest(int **arr, int n) {
    *arr = malloc(n * sizeof(int));
    buildBest(*arr, n, 0);
}