
graphs_add = lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png
graphs_list = lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png
source = lab2_add.c lab2_list.c SortedList.c SortedList.h
csv = lab2_add.csv lab2_list.csv
.SILENT:

all: build

build: lab2_add lab2_list

lab2_add: lab2_add.c
	gcc -Wall -Wextra lab2_add.c -o lab2_add -lrt -pthread

lab2_list: SortedList.c lab2_list.c
	gcc -Wall -Wextra SortedList.c lab2_list.c -o lab2_list -lrt -pthread

tests: build
	./add.sh
	./list.sh

graphs: clean-all tests
	./lab2_add.gp
	./lab2_list.gp

dist: graphs
	tar -czvf lab2a-604815835.tar.gz $(source) *.gp $(graphs_add) $(graphs_list) $(csv) add.sh list.sh Makefile README 

clean:
	rm -f lab2_add lab2_list lab2a-604815835.tar.gz

clean-all:
	rm -f lab2_add lab2_list lab2a-604815835.tar.gz *.csv *.png

