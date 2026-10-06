set encoding utf8
set terminal pngcairo size 1024,768 font 'Arial,10'
set output 'graphiques/20261006_211417/configurations/decroissant.png'
set datafile missing 'non_mesure'
set datafile separator ';'
set title 'Comparaison - Tableau inverse'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (secondes)'
set grid
set yrange [0:*]
plot 'resultats/20261006_211417/decroissant.csv' every ::1 using 1:2 title 'Tri par selection' with linespoints lc rgb '#d62728', 'resultats/20261006_211417/decroissant.csv' every ::1 using 1:4 title 'Tri a bulles' with linespoints lc rgb '#ff7f0e', 'resultats/20261006_211417/decroissant.csv' every ::1 using 1:6 title 'Tri par insertion' with linespoints lc rgb '#9467bd', 'resultats/20261006_211417/decroissant.csv' every ::1 using 1:8 title 'Tri rapide' with linespoints lc rgb '#1f77b4', 'resultats/20261006_211417/decroissant.csv' every ::1 using 1:10 title 'Tri fusion' with linespoints lc rgb '#2ca02c', 'resultats/20261006_211417/decroissant.csv' every ::1 using 1:12 title 'Tri par tas' with linespoints lc rgb '#8c564b'
unset output
