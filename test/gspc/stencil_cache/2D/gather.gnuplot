set grid
set xlabel "M/H"
set ylabel "Load efficiency"
set yrange [0:1]
set key below
set terminal png enhanced truecolor linewidth 3 fontscale 1.25 size 1920,1200

set title "Radius 2, #compute 5, #load 5, H = 2*(Radius-1)+1"
set xrange [0:609]
set xtics ("3" 9, "6" 18, "" 144, "51" 153, "" 297, "102" 306, "" 450, "153" 459, "" 597, "201" 603)
set ytics ("0" 0, "1/H" 0.3333, "1" 1)

set output "gather.2.png"
plot \
     "gather.2.50.50.out" u 10:(($4*$6)/$18) w lp lt 1 title "50 x 50" ,\
     "gather.2.100.50.out" u 10:(($4*$6)/$18) w lp lt 1 title "100 x 50" ,\
     "gather.2.150.50.out" u 10:(($4*$6)/$18) w lp lt 1 title "150 x 50" ,\
     "gather.2.200.50.out" u 10:(($4*$6)/$18) w lp lt 1 title "200 x 50" ,\
     "gather.2.50.100.out" u 10:(($4*$6)/$18) w lp lt 2 title "50 x 100" ,\
     "gather.2.100.100.out" u 10:(($4*$6)/$18) w lp lt 2 title "100 x 100" ,\
     "gather.2.150.100.out" u 10:(($4*$6)/$18) w lp lt 2 title "150 x 100" ,\
     "gather.2.200.100.out" u 10:(($4*$6)/$18) w lp lt 2 title "200 x 100" ,\
     "gather.2.50.150.out" u 10:(($4*$6)/$18) w lp lt 3 title "50 x 150" ,\
     "gather.2.100.150.out" u 10:(($4*$6)/$18) w lp lt 3 title "100 x 150" ,\
     "gather.2.150.150.out" u 10:(($4*$6)/$18) w lp lt 3 title "150 x 150" ,\
     "gather.2.200.150.out" u 10:(($4*$6)/$18) w lp lt 3 title "200 x 150" ,\
     "gather.2.50.200.out" u 10:(($4*$6)/$18) w lp lt 4 title "50 x 200" ,\
     "gather.2.100.200.out" u 10:(($4*$6)/$18) w lp lt 4 title "100 x 200" ,\
     "gather.2.150.200.out" u 10:(($4*$6)/$18) w lp lt 4 title "150 x 200" ,\
     "gather.2.200.200.out" u 10:(($4*$6)/$18) w lp lt 4 title "200 x 200"

set title "Radius 3, #compute 5, #load 5, H = 2*(Radius-1)+1"
set xrange [0:1050]
set xtics ("5" 25, "10" 50, "50" 250, "55" 275, "100" 500, "105" 525, "150" 750, "155" 775, "200" 1000, "205" 1025)
set ytics ("0" 0, "1/H" 0.2, "1" 1)

set output "gather.3.png"
plot \
     "gather.3.50.50.out" u 10:(($4*$6)/$18) w lp lt 1 title "50 x 50" ,\
     "gather.3.100.50.out" u 10:(($4*$6)/$18) w lp lt 1 title "100 x 50" ,\
     "gather.3.150.50.out" u 10:(($4*$6)/$18) w lp lt 1 title "150 x 50" ,\
     "gather.3.200.50.out" u 10:(($4*$6)/$18) w lp lt 1 title "200 x 50" ,\
     "gather.3.50.100.out" u 10:(($4*$6)/$18) w lp lt 2 title "50 x 100" ,\
     "gather.3.100.100.out" u 10:(($4*$6)/$18) w lp lt 2 title "100 x 100" ,\
     "gather.3.150.100.out" u 10:(($4*$6)/$18) w lp lt 2 title "150 x 100" ,\
     "gather.3.200.100.out" u 10:(($4*$6)/$18) w lp lt 2 title "200 x 100" ,\
     "gather.3.50.150.out" u 10:(($4*$6)/$18) w lp lt 3 title "50 x 150" ,\
     "gather.3.100.150.out" u 10:(($4*$6)/$18) w lp lt 3 title "100 x 150" ,\
     "gather.3.150.150.out" u 10:(($4*$6)/$18) w lp lt 3 title "150 x 150" ,\
     "gather.3.200.150.out" u 10:(($4*$6)/$18) w lp lt 3 title "200 x 150" ,\
     "gather.3.50.200.out" u 10:(($4*$6)/$18) w lp lt 4 title "50 x 200" ,\
     "gather.3.100.200.out" u 10:(($4*$6)/$18) w lp lt 4 title "100 x 200" ,\
     "gather.3.150.200.out" u 10:(($4*$6)/$18) w lp lt 4 title "150 x 200" ,\
     "gather.3.200.200.out" u 10:(($4*$6)/$18) w lp lt 4 title "200 x 200"
