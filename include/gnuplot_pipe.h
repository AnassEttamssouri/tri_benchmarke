#ifndef GNUPLOT_PIPE_H
#define GNUPLOT_PIPE_H

#include <stdio.h>
#include "sort.h"

FILE* gnuplot_init(void);
void gnuplot_close(FILE *gp);

void create_directories(void);

void generate_global_comparaison_plot(FILE* gp,char* filename);
void plot_algorithm_cases(FILE *gp,const char *algo_name);



#endif