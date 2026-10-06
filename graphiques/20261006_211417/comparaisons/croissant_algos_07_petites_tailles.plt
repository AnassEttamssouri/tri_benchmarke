set encoding utf8
set terminal pngcairo size 1024,768 font 'Arial,10'
set output 'graphiques/20261006_211417/comparaisons/croissant_algos_07_petites_tailles.png'
set datafile missing 'non_mesure'
set datafile separator ';'
set title 'Comparaison - Tableau deja trie - tailles jusqu a 5000'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (secondes)'
set grid
set yrange [0:*]
set xrange [0:5000]
plot 'resultats/20261006_211417/croissant.csv' every ::1 using 1:2 title 'Tri par selection' with linespoints lc rgb '#d62728', 'resultats/20261006_211417/croissant.csv' every ::1 using 1:4 title 'Tri a bulles' with linespoints lc rgb '#ff7f0e', 'resultats/20261006_211417/croissant.csv' every ::1 using 1:6 title 'Tri par insertion' with linespoints lc rgb '#9467bd'
unset output
