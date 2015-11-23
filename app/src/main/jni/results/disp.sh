#!/bin/bash
trace1="$1"
trace2="$2"

gnuplot <<- EOF
    set term png size 700,400 
    set output "$3.png"
    set origin 0,0
    set multiplot layout 2,1
    set grid
    set style fill solid border -1
    plot "$trace1" w l notitle
    plot "$trace2" w l notitle
    unset multiplot
EOF

