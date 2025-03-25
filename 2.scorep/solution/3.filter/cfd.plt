 set size square
 set key off
 unset xtics
 unset ytics
 set xrange [  -9: 650]
 set yrange [  -9: 650]
 plot "colourmap.dat" w rgbimage, "velocity.dat" u 1:2:(10*0.75*$3/sqrt($3**2+$4**2)):(10*0.75*$4/sqrt($3**2+$4**2)) with vectors  lc rgb "#7F7F7F"
