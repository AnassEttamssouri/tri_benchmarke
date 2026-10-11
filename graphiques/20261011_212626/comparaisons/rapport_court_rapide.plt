set encoding utf8
set terminal pngcairo size 1700,510 font 'Arial,19'
set output 'C:/Users/Waxes/Documents/ENSET/ProgrammationC/Rapports/RapportProjetTri/Images/rapport_court_20261011_212626_rapide.png'
set datafile missing 'non_mesure'
set title 'Tri rapide - entrées et stratégies de pivot'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (s)'
set logscale y 10
set format y '%.3g'
set grid ytics mytics
set key outside top center horizontal maxcols 4 font ',17'
set lmargin 10
set rmargin 7
set xrange [0:100000]
set yrange [*:*]
plot 'C:/Users/Waxes/Documents/ENSET/ProgrammationC/Projets/tri_benchmarke/resultats/20261011_212626/cases/quick_sort_best_avg_worst_cases.dat' using 1:($1<=100000 && column(2)>0 ? column(2) : 1/0) title 'Équilibré (pivot milieu)' with linespoints lw 2.4 pt 7 ps 0.7 lc rgb '#d62728', 'C:/Users/Waxes/Documents/ENSET/ProgrammationC/Projets/tri_benchmarke/resultats/20261011_212626/cases/quick_sort_best_avg_worst_cases.dat' using 1:($1<=100000 && column(3)>0 ? column(3) : 1/0) title 'Aléatoire (pivot aléatoire)' with linespoints lw 2.4 pt 7 ps 0.7 lc rgb '#ff7f0e', 'C:/Users/Waxes/Documents/ENSET/ProgrammationC/Projets/tri_benchmarke/resultats/20261011_212626/cases/quick_sort_best_avg_worst_cases.dat' using 1:($1<=100000 && column(4)>0 ? column(4) : 1/0) title 'Tout égal (pivot aléatoire)' with linespoints lw 2.4 pt 7 ps 0.7 lc rgb '#9467bd'
unset output
