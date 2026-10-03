#include "gnuplot_pipe.h"
#include "sort.h"
#include "benchmark.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>

FILE* gnuplot_init(){
    FILE* gp = _popen("gnuplot -persist","w");
    return gp;
}
void gnuplot_close(FILE* gp){
    fflush(gp);
    _pclose(gp);
}




void create_directories(void)
{


    _mkdir("results");
    _mkdir("plots");
}


void generate_global_comparaison_plot(FILE* gp,char* filename){
    fprintf(gp, "set terminal pngcairo size 1024,768 font 'Arial,10'\n");

    fprintf(gp, "set output 'plots/algorithm_benchmark/algorithm_benchmark.png'\n");

    if(gp == NULL){
        perror("Error opening GnuPlot");
        exit(1);
    }
    fprintf(gp, "set title 'Sorting Algorithm Performance'\n");
    fprintf(gp, "set xlabel 'Array Size (N)'\n");
    fprintf(gp, "set ylabel 'Time (seconds)'\n");
    fprintf(gp, "set grid\n");
    // Plot data from your generated file
    fprintf(gp, "plot '%s' using 1:2 title 'Selection Sort' with linespoints,"
    "'%s' using 1:3 title 'Bubble Sort' with linespoints,"
    "'%s' using 1:4 title 'Insertion Sort' with linespoints,"
    "'%s' using 1:5 title 'Quick Sort' with linespoints,"
    "'%s' using 1:6 title 'Merge Sort' with linespoints\n",filename,filename,filename,filename,filename);
    fprintf(gp, "unset output\n");
}




void plot_algorithm_cases(
    FILE *gp,
    const char *algo_name
)
{
    if (gp == NULL)
    {
        fprintf(stderr, "Échec d'ouverture du pipeline Gnuplot\n");
        exit(1);
    }

    fprintf(gp, "set terminal pngcairo size 1024,768 font 'Arial,10'\n");

    fprintf(
        gp,
        "set output 'plots/cases/%s_cases.png'\n",
        algo_name
    );

    fprintf(gp, "set title '%s - Comparaison des 3 cas'\n", algo_name);
    fprintf(gp, "set xlabel 'Taille du tableau'\n");
    fprintf(gp, "set ylabel 'Durée de tri (secondes)'\n");
    fprintf(gp, "set grid\n");

    fprintf(
        gp,
        "plot "
        "'results/cases/%s_cases.dat' using 1:2 "
        "with linespoints pt 7 "
        "title 'Cas simple (tableau trié)', "
        
        "'results/cases/%s_cases.dat' using 1:3 "
        "with linespoints pt 5 "
        "title 'Cas moyen (tableau aléatoire)', "
        
        "'results/cases/%s_cases.dat' using 1:4 "
        "with linespoints pt 9 "
        "title 'Cas difficile (tableau inversé)'\n",
        
        algo_name,
        algo_name,
        algo_name
    );

    fflush(gp);

    fprintf(gp, "unset output\n");
}