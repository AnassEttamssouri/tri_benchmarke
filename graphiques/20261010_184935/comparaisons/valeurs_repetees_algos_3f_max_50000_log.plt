set encoding utf8
set terminal pngcairo size 1024,768 font 'Arial,10'
set output 'graphiques/20261010_184935/comparaisons/valeurs_repetees_algos_3f_max_50000_log.png'
set datafile missing 'non_mesure'
set datafile separator ';'
set title 'Comparaison - Dix valeurs repetees - N <= 50000'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (secondes, echelle logarithmique)'
set logscale y 10
set format y '%.3g'
set grid ytics mytics
set key outside top center horizontal maxcols 3
set xrange [0:50000]
plot 'resultats/20261010_184935/valeurs_repetees.csv' every ::1 using 1:(column(1)<=50000 && column(2)>0 ? column(2) : 1/0) title 'Tri par selection' with linespoints lc rgb '#d62728', 'resultats/20261010_184935/valeurs_repetees.csv' every ::1 using 1:(column(1)<=50000 && column(4)>0 ? column(4) : 1/0) title 'Tri a bulles' with linespoints lc rgb '#ff7f0e', 'resultats/20261010_184935/valeurs_repetees.csv' every ::1 using 1:(column(1)<=50000 && column(6)>0 ? column(6) : 1/0) title 'Tri par insertion' with linespoints lc rgb '#9467bd', 'resultats/20261010_184935/valeurs_repetees.csv' every ::1 using 1:(column(1)<=50000 && column(8)>0 ? column(8) : 1/0) title 'Tri rapide' with linespoints lc rgb '#1f77b4', 'resultats/20261010_184935/valeurs_repetees.csv' every ::1 using 1:(column(1)<=50000 && column(10)>0 ? column(10) : 1/0) title 'Tri fusion' with linespoints lc rgb '#2ca02c', 'resultats/20261010_184935/valeurs_repetees.csv' every ::1 using 1:(column(1)<=50000 && column(12)>0 ? column(12) : 1/0) title 'Tri par tas' with linespoints lc rgb '#8c564b'
unset output
