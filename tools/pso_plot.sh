#!/bin/sh
tail -n $2 /boinc/milkyway/log_milkyway/$1 > /boinc/milkyway/log_milkyway/plot_input
gnuplot << EOF
reset
set key outside center bottom horizontal
set terminal jpeg giant
set title "Particle Swarm Optimization: $1"
set ylabel "Fitness"
set xlabel "Particles Analyzed"
set pointsize 0.3
set output "/boinc/milkyway/download/plots/$1.jpeg"
plot [$3][$4] "/boinc/milkyway/log_milkyway/plot_input" using 1:2 '%lf -- b: %lf, a: %lf, w: %lf' title 'best' w points, \
	"/boinc/milkyway/log_milkyway/plot_input" using 1:3 '%lf -- b: %lf, a: %lf, w: %lf' title 'average' w points, \
	"/boinc/milkyway/log_milkyway/plot_input" using 1:4 '%lf -- b: %lf, a: %lf, w: %lf' title 'worst' w points
