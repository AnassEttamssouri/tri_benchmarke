set encoding utf8
set terminal pngcairo size 1024,768 font 'Arial,10'
set output 'graphiques/20261010_184935/comparaisons/experience_rapide_max_100000_log.png'
set datafile missing 'non_mesure'
set title 'Tri rapide - Partitions equilibrees, aleatoire et valeurs egales'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (secondes, echelle logarithmique)'
set logscale y 10
set format y '%.3g'
set grid ytics mytics
set key outside top center horizontal maxcols 3
set xrange [0:100000]
plot 'resultats/20261010_184935/cases/quick_sort_best_avg_worst_cases.dat' using 1:(column(1)<=100000 && column(2)>0 ? column(2) : 1/0) title 'Partitions equilibrees (pivot milieu)' with linespoints lc rgb '#d62728', 'resultats/20261010_184935/cases/quick_sort_best_avg_worst_cases.dat' using 1:(column(1)<=100000 && column(3)>0 ? column(3) : 1/0) title 'Tableau aleatoire (pivot aleatoire)' with linespoints lc rgb '#ff7f0e', 'resultats/20261010_184935/cases/quick_sort_best_avg_worst_cases.dat' using 1:(column(1)<=100000 && column(4)>0 ? column(4) : 1/0) title 'Toutes les valeurs egales (pivot aleatoire)' with linespoints lc rgb '#9467bd'
unset output
