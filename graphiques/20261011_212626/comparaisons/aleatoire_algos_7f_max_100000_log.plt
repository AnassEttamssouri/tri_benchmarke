set encoding utf8
set terminal pngcairo size 1024,768 font 'Arial,10'
set output 'graphiques/20261011_212626/comparaisons/aleatoire_algos_7f_max_100000_log.png'
set datafile missing 'non_mesure'
set datafile separator ';'
set title 'Comparaison - Tableau aleatoire - N <= 100000'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (secondes, echelle logarithmique)'
set logscale y 10
set format y '%.3g'
set grid ytics mytics
set key outside top center horizontal maxcols 3
set xrange [0:100000]
plot 'resultats/20261011_212626/aleatoire.csv' every ::1 using 1:(column(1)<=100000 && column(2)>0 ? column(2) : 1/0) title 'Tri par selection' with linespoints lc rgb '#d62728', 'resultats/20261011_212626/aleatoire.csv' every ::1 using 1:(column(1)<=100000 && column(4)>0 ? column(4) : 1/0) title 'Tri a bulles' with linespoints lc rgb '#ff7f0e', 'resultats/20261011_212626/aleatoire.csv' every ::1 using 1:(column(1)<=100000 && column(6)>0 ? column(6) : 1/0) title 'Tri par insertion' with linespoints lc rgb '#9467bd', 'resultats/20261011_212626/aleatoire.csv' every ::1 using 1:(column(1)<=100000 && column(8)>0 ? column(8) : 1/0) title 'Tri rapide' with linespoints lc rgb '#1f77b4', 'resultats/20261011_212626/aleatoire.csv' every ::1 using 1:(column(1)<=100000 && column(10)>0 ? column(10) : 1/0) title 'Tri fusion' with linespoints lc rgb '#2ca02c', 'resultats/20261011_212626/aleatoire.csv' every ::1 using 1:(column(1)<=100000 && column(12)>0 ? column(12) : 1/0) title 'Tri par tas' with linespoints lc rgb '#8c564b', 'resultats/20261011_212626/aleatoire.csv' every ::1 using 1:(column(1)<=100000 && column(14)>0 ? column(14) : 1/0) title 'Tri Shell' with linespoints lc rgb '#e377c2'
unset output
