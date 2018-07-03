
default: build

build:
	gcc lab3a.c -Wall -Wextra -o lab3a

source = README Makefile lab3a.c ext2_fs.h
lab3a-404796809.tar.gz: $(source)
	tar -czf $@ $(source)

clean:
	rm -rf *.o *.so lab3a-404796809.tar.gz lab3a

dist: lab3a-404796809.tar.gz

.PHONY: default clean dist
