

#ifndef BENCHMARK_H
#define BENCHMARK_H
#include "sort.h"

    void generate_global_comparaison(char* filename,int* sizes,int count);
    double measure_time(sort_func algorithm, int* arr, int n);
    
    void benchmark_algorithm_cases(const char *filename,sort_func algo,int *sizes,int count);
    void benchmark_algorithm_quick(const char *filename,int *sizes,int count);

    int generate_sizes(int start_size  , int jump , int end_size, int ** sizes);
    void generateRandom(int **arr, int n);
    void generateSorted(int **arr, int n);
    void generateReversed(int **arr, int n);
    void generateAllEqual(int **arr, int n);
    void generateQuickBest(int **arr, int n);

#endif
