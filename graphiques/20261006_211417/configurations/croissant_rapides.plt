set encoding utf8
set terminal pngcairo size 1024,768 font 'Arial,10'
set output 'graphiques/20261006_211417/configurations/croissant_rapides.png'
set datafile missing 'non_mesure'
set datafile separator ';'
set title 'Comparaison - Tableau deja trie - rapide, fusion et tas'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (secondes)'
set grid
set yrange [0:*]
plot 'resultats/20261006_211417/croissant.csv' every ::1 using 1:8 title 'Tri rapide' with linespoints lc rgb '#1f77b4', 'resultats/20261006_211417/croissant.csv' every ::1 using 1:10 title 'Tri fusion' with linespoints lc rgb '#2ca02c', 'resultats/20261006_211417/croissant.csv' every ::1 using 1:12 title 'Tri par tas' with linespoints lc rgb '#8c564b'
unset output
