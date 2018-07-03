
.SILENT:

build: clean lab3b

lab3b: lab3b.py
	cp lab3b.py lab3b
	chmod +x lab3b

clean:
	rm -f lab3b lab3b-604815835.tar.gz

dist: clean lab3b
	tar -czvf lab3b-604815835.tar.gz lab3b.py Makefile README
