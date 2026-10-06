set encoding utf8
set terminal pngcairo size 1024,768 font 'Arial,10'
set output 'graphiques/20261006_211417/cases/heap_sort_cases.png'
set datafile missing 'non_mesure'
set title 'Tri par tas - Comparaison des configurations'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (secondes)'
set grid
set yrange [0:*]
plot 'resultats/20261006_211417/cases/heap_sort_cases.dat' using 1:2 title 'Tableau trie' with linespoints lc rgb '#d62728', 'resultats/20261006_211417/cases/heap_sort_cases.dat' using 1:3 title 'Tableau aleatoire' with linespoints lc rgb '#ff7f0e', 'resultats/20261006_211417/cases/heap_sort_cases.dat' using 1:4 title 'Tableau inverse' with linespoints lc rgb '#9467bd', 'resultats/20261006_211417/cases/heap_sort_cases.dat' using 1:5 title 'Tableau presque trie' with linespoints lc rgb '#1f77b4', 'resultats/20261006_211417/cases/heap_sort_cases.dat' using 1:6 title 'Dix valeurs repetees' with linespoints lc rgb '#2ca02c'
unset output
