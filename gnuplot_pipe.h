#ifndef GNUPLOT_PIPE_H
#define GNUPLOT_PIPE_H

#include <stdio.h>
#include "sort.h"

int generate_sizes(int start_size, int jump, int end_size, int **sizes);
void create_directories(void);

void generate_global_comparaison_plot(FILE* gp,char* filename,int* sizes,int count);
void plot_algorithm_cases(FILE *gp,const char *algo_name);
FILE* gnuplot_init(void);
void gnuplot_close(FILE *gp);

#endif