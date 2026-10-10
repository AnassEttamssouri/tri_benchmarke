set encoding utf8
set terminal pngcairo size 1024,768 font 'Arial,10'
set output 'graphiques/20261010_184935/configurations/aleatoire_petites_tailles.png'
set datafile missing 'non_mesure'
set datafile separator ';'
set title 'Comparaison - Tableau aleatoire - tailles jusqu a 5000'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (secondes)'
set grid
set yrange [0:*]
set xrange [0:5000]
plot 'resultats/20261010_184935/aleatoire.csv' every ::1 using 1:(column(1)<=5000 && column(2)>=0 ? column(2) : 1/0) title 'Tri par selection' with linespoints lc rgb '#d62728', 'resultats/20261010_184935/aleatoire.csv' every ::1 using 1:(column(1)<=5000 && column(4)>=0 ? column(4) : 1/0) title 'Tri a bulles' with linespoints lc rgb '#ff7f0e', 'resultats/20261010_184935/aleatoire.csv' every ::1 using 1:(column(1)<=5000 && column(6)>=0 ? column(6) : 1/0) title 'Tri par insertion' with linespoints lc rgb '#9467bd', 'resultats/20261010_184935/aleatoire.csv' every ::1 using 1:(column(1)<=5000 && column(8)>=0 ? column(8) : 1/0) title 'Tri rapide' with linespoints lc rgb '#1f77b4', 'resultats/20261010_184935/aleatoire.csv' every ::1 using 1:(column(1)<=5000 && column(10)>=0 ? column(10) : 1/0) title 'Tri fusion' with linespoints lc rgb '#2ca02c', 'resultats/20261010_184935/aleatoire.csv' every ::1 using 1:(column(1)<=5000 && column(12)>=0 ? column(12) : 1/0) title 'Tri par tas' with linespoints lc rgb '#8c564b'
unset output
