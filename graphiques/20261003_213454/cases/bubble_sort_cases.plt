set encoding utf8
set terminal pngcairo size 1024,768 font 'Arial,10'
set output 'graphiques/20261003_213454/cases/bubble_sort_cases.png'
set datafile missing 'non_mesure'
set title 'Tri a bulles - Comparaison des configurations'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (secondes)'
set grid
set yrange [0:*]
plot 'resultats/20261003_213454/cases/bubble_sort_cases.dat' using 1:2 title 'Tableau trie' with linespoints
unset output