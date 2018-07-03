
.SILENT:

lab1a: lab1a.c
	gcc -Wall -Wextra lab1a.c -o lab1a

clean:
	rm -f lab1a lab1a-604815835.tar.gz

dist: lab1a
	tar -czvf lab1a-604815835.tar.gz lab1a.c Makefile README
