set encoding utf8
set terminal pngcairo size 1400,900 font 'Arial,12'
set output 'graphiques/20261006_211417/histogrammes/experience_rapide_1000000.png'
set datafile separator ';'
set datafile missing 'non_mesure'
set title 'Experience tri rapide : strategies de pivot indiquees - N=1000000 - run 20261006_211417'
set ylabel 'Temps de tri (secondes)'
set style data histograms
set style histogram clustered gap 1
set style fill solid 0.85 border -1
set boxwidth 0.85
set grid ytics
set key outside top center horizontal
set xtics rotate by -15
set bmargin 7
set yrange [0:0.32236801575]
set label 1 'Minimum des repetitions et moyenne arithmetique; aucune nouvelle mesure.' at graph 0.01,0.97 front
set label 2 'Non mesure : Toutes egales (pivot aleatoire) (limite lente)' at graph 0.01,0.910 front
plot 'graphiques/20261006_211417/histogrammes/experience_rapide_1000000.dat' every ::1 using 2:xticlabels(1) title 'Minimum (s)' lc rgb '#1f77b4', '' every ::1 using 3 title 'Moyenne (s)' lc rgb '#ff7f0e', '' every ::1 using ($0-0.17):2:(sprintf('%.6g s',column(2))) with labels rotate by 90 left offset 0,1 notitle, '' every ::1 using ($0+0.17):3:(sprintf('%.6g s',column(3))) with labels rotate by 90 left offset 0,1 notitle
unset output
