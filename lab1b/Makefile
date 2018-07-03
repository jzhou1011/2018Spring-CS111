
.SILENT:

all: lab1b-client lab1b-server

lab1b-server: lab1b-server.c
	gcc -Wno-pointer-sign -Wall -Wextra lab1b-server.c -o lab1b-server -lz

lab1b-client: lab1b-client.c
	gcc -Wno-pointer-sign -Wall -Wextra lab1b-client.c -o lab1b-client -lz

clean:
	rm -f lab1b-server lab1b-client lab1b-604815835.tar.gz

dist: lab1b-server lab1b-client
	tar -czvf lab1b-604815835.tar.gz lab1b-server.c lab1b-client.c Makefile README
