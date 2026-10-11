set encoding utf8
set terminal pngcairo size 1700,500 font 'Arial,19'
set output 'C:/Users/Waxes/Documents/ENSET/ProgrammationC/Rapports/RapportProjetTri/Images/rapport_court_20261011_212626_histogrammes.png'
set datafile separator ';'
set multiplot layout 1,2
set style data histograms
set style histogram clustered gap 1
set style fill solid 0.85 border -1
set boxwidth 0.85
set grid ytics
set key outside top center horizontal font ',17'
set ylabel 'Temps (s)'
set xtics rotate by -25
set bmargin 5
set lmargin 9
set rmargin 2
set format y '%.3g'
set title '(a) Sept tris, N = 100 000'
set yrange [0:34.549943436]
plot 'C:/Users/Waxes/Documents/ENSET/ProgrammationC/Projets/tri_benchmarke/graphiques/20261011_212626/histogrammes/rapport_aleatoire_100000.dat' every ::1 using 2:xticlabels(1) title 'Minimum' lc rgb '#1f77b4', '' every ::1 using 3 title 'Moyenne' lc rgb '#ff7f0e', '' every ::1 using ($0-0.18):2:(sprintf('%.3g',column(2))) with labels rotate by 90 left offset 0,0.7 font ',14' notitle, '' every ::1 using ($0+0.18):3:(sprintf('%.3g',column(3))) with labels rotate by 90 left offset 0,0.7 font ',14' notitle
set title '(b) Quatre tris rapides, même entrée'
set yrange [0:0.022662792]
plot 'C:/Users/Waxes/Documents/ENSET/ProgrammationC/Projets/tri_benchmarke/graphiques/20261011_212626/histogrammes/rapport_aleatoire_100000.dat' every ::4 using 2:xticlabels(1) title 'Minimum' lc rgb '#1f77b4', '' every ::4 using 3 title 'Moyenne' lc rgb '#ff7f0e', '' every ::4 using ($0-0.18):2:(sprintf('%.3g',column(2))) with labels rotate by 90 left offset 0,0.7 font ',14' notitle, '' every ::4 using ($0+0.18):3:(sprintf('%.3g',column(3))) with labels rotate by 90 left offset 0,0.7 font ',14' notitle
unset multiplot
unset output
