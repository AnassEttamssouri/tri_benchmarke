set encoding utf8
set terminal pngcairo size 1024,768 font 'Arial,10'
set output 'graphiques/20261006_211417/comparaisons/aleatoire_algos_38_petites_tailles.png'
set datafile missing 'non_mesure'
set datafile separator ';'
set title 'Comparaison - Tableau aleatoire - tailles jusqu a 5000'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (secondes)'
set grid
set yrange [0:*]
set xrange [0:5000]
plot 'resultats/20261006_211417/aleatoire.csv' every ::1 using 1:8 title 'Tri rapide' with linespoints lc rgb '#1f77b4', 'resultats/20261006_211417/aleatoire.csv' every ::1 using 1:10 title 'Tri fusion' with linespoints lc rgb '#2ca02c', 'resultats/20261006_211417/aleatoire.csv' every ::1 using 1:12 title 'Tri par tas' with linespoints lc rgb '#8c564b'
unset output
