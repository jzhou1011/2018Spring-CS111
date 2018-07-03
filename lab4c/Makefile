
.SILENT:

build: clean lab4c_tcp lab4c_tls

lab4c_tcp: lab4c_tcp.c
	#gcc -Wall -Wextra lab4c_tcp.c -o lab4c_tcp -g -DDUMMY -lm
	gcc -Wall -Wextra lab4c_tcp.c -o lab4c_tcp -g -lmraa -lm

lab4c_tls: lab4c_tls.c
	#gcc -Wall -Wextra lab4c_tls.c -o lab4c_tls -g -DDUMMY -lm -lssl -lcrypto
	gcc -Wall -Wextra lab4c_tls.c -o lab4c_tls -g -lmraa -lm -lssl -lcrypto

clean:
	rm -f lab4c_tls lab4c_tcp lab4c-604815835.tar.gz

dist: clean build
	tar -czvf lab4c-604815835.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README
