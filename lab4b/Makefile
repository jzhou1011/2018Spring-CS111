
.SILENT:

build: clean lab4b

lab4b: lab4b.c
	#gcc -Wall -Wextra lab4b.c -o lab4b -g -DDUMMY -lm
	gcc -Wall -Wextra lab4b.c -o lab4b -g -lmraa -lm

check: lab4b
	chmod +x test.sh
	./test.sh

clean:
	rm -f lab4b lab4b-604815835.tar.gz STDIN STDOUT STDERR

dist: clean lab4b
	tar -czvf lab4b-604815835.tar.gz lab4b.c Makefile README test.sh
