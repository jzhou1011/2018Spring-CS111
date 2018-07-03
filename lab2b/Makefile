
graphs = lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png
source = lab2_list.c SortedList.c SortedList.h
override CFLAGS += -pthread -g
.SILENT:


all: build

build: lab2_list

lab2_list: clean-all SortedList.o lab2_list.o
	gcc -Wall -Wextra -o $@ -pthread -g -L /u/cs/ugrad/jinjing/lib -lprofiler SortedList.o lab2_list.o -lrt
	#gcc -Wall -Wextra -o lab2_list -pthread -g SortedList.o lab2_list.o -lrt

tests: build
	chmod +x lab2b.sh
	./lab2b.sh


graphs: tests
	chmod +x lab2b.gp
	./lab2b.gp

profile: profile.gperf

profile.gperf: build
	rm -f ./raw.gperf
	LD_PRELOAD=/u/cs/ugrad/jinjing/lib/libprofiler.so.0 CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.gperf
	pprof --list=thread_func ./lab2_list ./raw.gperf >> $@
	mv profile.gperf profile.out
	rm -f ./raw.gperf

dist: graphs profile
	tar -czvf lab2b-604815835.tar.gz $(source) lab2b.gp lab2b.sh lab2b_list.csv $(graphs) profile.out Makefile README 

clean:
	rm -f lab2_list *.o lab2b-604815835.tar.gz

clean-all:
	rm -f lab2_list *.csv *.o lab2b-604815835.tar.gz