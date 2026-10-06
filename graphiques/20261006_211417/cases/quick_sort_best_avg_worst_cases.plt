set encoding utf8
set terminal pngcairo size 1024,768 font 'Arial,10'
set output 'graphiques/20261006_211417/cases/quick_sort_best_avg_worst_cases.png'
set datafile missing 'non_mesure'
set title 'Tri rapide - Partitions equilibrees, aleatoire et valeurs egales'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (secondes)'
set grid
set yrange [0:*]
plot 'resultats/20261006_211417/cases/quick_sort_best_avg_worst_cases.dat' using 1:2 title 'Partitions equilibrees (pivot milieu)' with linespoints lc rgb '#d62728', 'resultats/20261006_211417/cases/quick_sort_best_avg_worst_cases.dat' using 1:3 title 'Tableau aleatoire (pivot aleatoire)' with linespoints lc rgb '#ff7f0e', 'resultats/20261006_211417/cases/quick_sort_best_avg_worst_cases.dat' using 1:4 title 'Toutes les valeurs egales (pivot aleatoire)' with linespoints lc rgb '#9467bd'
unset output
