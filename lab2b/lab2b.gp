#! /usr/local/cs/bin/gnuplot

#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2b_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#	8. avg wait time for locks

# general plot parameters
set terminal png
set datafile separator ","

set title "lab2b-1 Throughput of Synchronized Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operation/sec)"
set logscale y
set output 'lab2b_1.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'list ins/lookup/delete w/mutex' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'list ins/lookup/delete w/spin' with linespoints lc rgb 'green'

set title "lab2b-2 Per-operation Times for Mutex-Protected List Operations"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "mean time/operation"
set logscale y
set output 'lab2b_2.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'completion time' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'wait for lock' with linespoints lc rgb 'green'

set title "lab2b-3: Correct Synchronization of Partitioned Lists"
set xlabel "Number of threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Number of iterations"
set logscale y 10
set output 'lab2b_3.png'
set key left top
# grep out only successful (sum=0)  unsynchronized runs
plot \
     "< grep -e 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'mutex' with points lc rgb 'green', \
	 "< grep -e 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'spin-lock' with points lc rgb 'blue', \
     "< grep -e 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'unprotected' with points lc rgb 'red'

set title "lab2b-4 Throughput of Mutex-Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operation/sec)"
set logscale y
set output 'lab2b_4.png'
set key right top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'lists=1' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'lists=4' with linespoints lc rgb 'green', \
	 "< grep -e 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'lists=8' with linespoints lc rgb 'red', \
	 "< grep -e 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'lists=16' with linespoints lc rgb 'orange'

set title "lab2b-5 Throughput of Spinlock-Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operation/sec)"
set logscale y
set output 'lab2b_5.png'
set key right top
plot \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'lists=1' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'lists=4' with linespoints lc rgb 'green', \
	 "< grep -e 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'lists=8' with linespoints lc rgb 'red', \
	 "< grep -e 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'lists=16' with linespoints lc rgb 'orange'







