set encoding utf8
set terminal pngcairo size 1700,430 font 'Arial,19'
set output 'C:/Users/Waxes/Documents/ENSET/ProgrammationC/Rapports/RapportProjetTri/Images/rapport_court_20261011_212626_million.png'
set datafile missing 'non_mesure'
set title 'Entrée aléatoire - mesures jusqu’à 1 000 000'
set xlabel 'Taille du tableau (N)'
set ylabel 'Temps minimum (s)'
set logscale y 10
set format y '%.3g'
set grid ytics mytics
set key outside top center horizontal maxcols 4 font ',17'
set lmargin 10
set rmargin 7
set xrange [0:1000000]
set yrange [*:*]
set datafile separator ';'
plot 'C:/Users/Waxes/Documents/ENSET/ProgrammationC/Projets/tri_benchmarke/resultats/20261011_212626/aleatoire.csv' every ::1 using 1:($1<=1000000 && column(8)>0 ? column(8) : 1/0) title 'Rapide' with linespoints lw 2.4 pt 7 ps 0.7 lc rgb '#1f77b4', 'C:/Users/Waxes/Documents/ENSET/ProgrammationC/Projets/tri_benchmarke/resultats/20261011_212626/aleatoire.csv' every ::1 using 1:($1<=1000000 && column(10)>0 ? column(10) : 1/0) title 'Fusion' with linespoints lw 2.4 pt 7 ps 0.7 lc rgb '#2ca02c', 'C:/Users/Waxes/Documents/ENSET/ProgrammationC/Projets/tri_benchmarke/resultats/20261011_212626/aleatoire.csv' every ::1 using 1:($1<=1000000 && column(12)>0 ? column(12) : 1/0) title 'Tas' with linespoints lw 2.4 pt 7 ps 0.7 lc rgb '#8c564b', 'C:/Users/Waxes/Documents/ENSET/ProgrammationC/Projets/tri_benchmarke/resultats/20261011_212626/aleatoire.csv' every ::1 using 1:($1<=1000000 && column(14)>0 ? column(14) : 1/0) title 'Shell' with linespoints lw 2.4 pt 7 ps 0.7 lc rgb '#e377c2'
unset output
