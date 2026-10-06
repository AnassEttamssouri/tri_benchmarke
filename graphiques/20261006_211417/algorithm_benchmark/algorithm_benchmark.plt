set encoding utf8
set terminal pngcairo size 1024,768 font 'Arial,10'
set output 'graphiques/20261006_211417/algorithm_benchmark/algorithm_benchmark.png'
set datafile missing 'non_mesure'
set title 'Sorting Algorithm Performance'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (secondes)'
set grid
set yrange [0:*]
plot 'resultats/20261006_211417/benchmark/res.dat' using 1:2 title 'Tri par selection' with linespoints lc rgb '#d62728', 'resultats/20261006_211417/benchmark/res.dat' using 1:3 title 'Tri a bulles' with linespoints lc rgb '#ff7f0e', 'resultats/20261006_211417/benchmark/res.dat' using 1:4 title 'Tri par insertion' with linespoints lc rgb '#9467bd', 'resultats/20261006_211417/benchmark/res.dat' using 1:5 title 'Tri rapide' with linespoints lc rgb '#1f77b4', 'resultats/20261006_211417/benchmark/res.dat' using 1:6 title 'Tri fusion' with linespoints lc rgb '#2ca02c', 'resultats/20261006_211417/benchmark/res.dat' using 1:7 title 'Tri par tas' with linespoints lc rgb '#8c564b'
unset output
