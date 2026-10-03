#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sort.h"
#include "gnuplot_pipe.h"
#include "benchmark.h"

#define BENCHMARK_RESULTS_FILE     "./results/benchmark/res.dat"



int main(void)
{
    srand((unsigned int)time(NULL));


    int start_size = 10000;
    int jump = 1000;
    int end_size = 20000;

    create_directories();

    int *sizes = NULL;
    int num_sizes = generate_sizes(start_size, jump, end_size, &sizes);

    if (num_sizes == 0)
    {
        fprintf(stderr, "Erreur lors de la génération des tailles.\n");
        return 1;
    }
    FILE* gp = gnuplot_init();

    //géneration de la comparaison globale entre les algorithmes :
    generate_global_comparaison(BENCHMARK_RESULTS_FILE,sizes,num_sizes);
    generate_global_comparaison_plot(gp,BENCHMARK_RESULTS_FILE,sizes,num_sizes);

    //géneration des cas favorables , non favorables des algorithmes : 
    benchmark_algorithm_cases("./results/cases/selection_sort_cases.dat",selectionSort,sizes,num_sizes);
    benchmark_algorithm_cases("./results/cases/bubble_sort_cases.dat",bubbleSort,sizes,num_sizes);
    benchmark_algorithm_cases("./results/cases/insertion_sort_cases.dat",insertionSort,sizes,num_sizes);
    //choix du tableau trié , aléatoire , inversé
    benchmark_algorithm_cases("./results/cases/quick_sort_input_order_cases.dat",quickSort,sizes,num_sizes);
    //choix du tableau divisé en partition de même taille , tableau aléatoire , tableau avec meme valeur 
    benchmark_algorithm_quick("./results/cases/quick_sort_best_avg_worst_cases.dat",sizes,num_sizes);

    benchmark_algorithm_cases("./results/cases/merge_sort_cases.dat",mergeSort,sizes,num_sizes);

    plot_algorithm_cases(gp,"selection_sort");
    plot_algorithm_cases(gp,"bubble_sort");
    plot_algorithm_cases(gp,"insertion_sort");
    plot_algorithm_cases(gp,"quick_sort_input_order");
    plot_algorithm_cases(gp,"quick_sort_best_avg_worst");
    plot_algorithm_cases(gp,"merge_sort");


    //férmeture du pipeline vers GnuPlot :
    gnuplot_close(gp);

    return 0;

}