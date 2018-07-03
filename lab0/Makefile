

.SILENT:

lab0: lab0.c
	gcc -Wall -Wextra -g lab0.c -o lab0
check: lab0 test1.1 test2.1 test3.1 test3.2 test3.3 test4.1 test4.2 test5.1 test6.1 test7.1 cleantest

test1.1: test1.1.1 test1.1.2 test1.1.3

test2.1: test2.1.1 test2.1.2 test2.1.3

test3.1: test3.1.1 test3.1.2 test3.1.3

test3.2: test3.2.1 test3.2.2 test3.2.3

test3.3: test3.3.1 test3.3.2 test3.3.3

test4.1: test4.1.1 test4.1.2 test4.1.3

test4.2: test4.2.1 test4.2.2 test4.2.3

test5.1: test5.1.1 test5.1.2 test5.1.3

test7.1: test7.1.1 test7.1.2

cleantest:
	rm -f STDOUT STDERR STDIN cannot_open empty

dist: lab0
	tar -czvf lab0-604815835.tar.gz lab0.c Makefile README backtrace.png breakpoint.png
clean:
	rm -f lab0 STDOUT STDERR STDIN cannot_open empty lab0-604815835.tar.gz

# Smoke-tests to make sure program works. Called by make check

test1.1.1:
	echo "Test 1.1: Testing lab0 without any arguments..."; \
	touch STDIN; \
	echo "abcde" > STDIN; \
	./lab0 < STDIN > STDOUT 2>STDERR ; \
	if [[ $$? -eq 0 ]]; then \
	echo "	C: Correct Exit Code."; \
	else \
	echo "	W: Wrong Exit Code."; \
	fi
test1.1.2:
	cmp STDIN STDOUT &>/dev/null; \
	if [[ $$? -eq 0 ]]; then \
	echo "	C: Output matches input."; \
	else \
	echo "	W: Output does not match with input."; \
	fi
test1.1.3:
	touch empty; \
	cmp STDERR empty &>/dev/null; \
	if [[ $$? -eq 0 ]]; then \
	echo "	C: No error messsage."; \
	else \
	echo "	W: There should not be any error message."; \
	fi

test2.1.1:
	echo "Test 2.1: Testing unrecognized arguments..."; \
	./lab0 --wrongarg >STDOUT 2> STDERR; \
	if [[ $$? -eq 1 ]]; \
	then \
		echo "	C: Correct Exit Code."; \
	else \
		echo "	W: Wrong Exit Code."; \
	fi
test2.1.2:
	if [[ ! -s STDERR ]]; then \
		echo "	W: Should output error message when wrong arguments are used."; \
	else \
		echo "	C: Error message and Usage message shoule be outputted as follows:"; \
		cat STDERR; \
	fi
test2.1.3:
	if [[ ! -s STDOUT ]]; then \
		echo "	C: No output."; \
	else \
		echo "	W: Should not be any output."; \
	fi

test3.1.1:
	echo "Test 3.1: Testing with an input file given..."; \
	./lab0 --input="STDIN" > STDOUT 2> STDERR; \
	if [[ $$? -eq 0 ]]; \
	then \
		echo "	C: Correct Exit Code."; \
	else \
		echo "	W: Wrong Exit Code."; \
	fi
test3.1.2:
	if [[ ! -s STDERR ]]; then \
		echo "	C: No error messsage."; \
	else \
		echo "	W: There should not be any error message."; \
	fi
test3.1.3:
	cmp STDIN STDOUT &>/dev/null; \
	if [[ $$? -eq 0 ]]; \
	then \
		echo "	C: Output matches input."; \
	else \
		echo "	W: Output does not match with input."; \
	fi

test3.2.1:
	echo "Test 3.2: Testing with a non-existent input file given..."; \
	./lab0 --input="non_existent" > STDOUT 2> STDERR; \
	if [[ $$? -eq 2 ]]; \
	then \
		echo "	C: Correct Exit Code."; \
	else \
		echo "	W: Wrong Exit Code."; \
	fi
test3.2.2:
	if [[ ! -s STDERR ]]; \
	then \
		echo "	W: No error message."; \
	else \
		echo "	C: Error message printed as follows:"; \
		cat STDERR; \
	fi
test3.2.3:
	if [[ ! -s STDOUT ]]; \
	then \
		echo "	C: No output."; \
	else \
		echo "	W: Should not be any output."; \
		cat STDERR; \
	fi

test3.3.1:
	echo "Test 3.3: Testing with an input file that cannot be opened..."; \
	touch cannot_open; \
	chmod 0000 cannot_open; \
	./lab0 --input=cannot_open > STDOUT 2> STDERR; \
	if [[ $$? -eq 2 ]]; \
	then \
		echo "	C: Correct Exit Code."; \
	else \
		echo "	W: Wrong Exit Code."; \
	fi
test3.3.2:
	if [[ ! -s STDERR ]]; \
	then \
		echo "	W: No error message."; \
	else \
		echo "	C: Error message printed as follows:"; \
		cat STDERR; \
	fi
test3.3.3:
	if [[ ! -s STDOUT ]]; \
	then \
		echo "	C: No output."; \
	else \
		echo "	W: Should not be any output."; \
	fi

test4.1.1:
	echo "Test 4.1: Testing with an output file given..."; \
	./lab0 --output="STDOUT" <STDIN 2> STDERR; \
	if [[ $$? -eq 0 ]]; \
	then \
		echo "	C: Correct Exit Code."; \
	else \
		echo "	W: Wrong Exit Code."; \
	fi
test4.1.2:
	cmp STDIN STDOUT &> /dev/null; \
	if [[ $$? -eq 0 ]]; \
	then \
		echo "	C: Output matches input."; \
	else \
		echo "	W: Output does not match with input."; \
	fi
test4.1.3:
	if [[ ! -s STDERR ]]; \
	then \
		echo "	C: No error messsage."; \
	else \
		echo "	W: There should not be any error message."; \
	fi

test4.2.1:
	echo "Test 4.2: Testing with an output file that cannot be opened given..."; \
	./lab0 --output="cannot_open" <STDIN >STDOUT 2> STDERR; \
	if [[ $$? -eq 3 ]]; \
	then \
		echo "	C: Correct Exit Code."; \
	else \
		echo "	W: Wrong Exit Code."; \
	fi
test4.2.2:
	if [[ ! -s STDERR ]]; \
	then \
		echo "	W: No error message."; \
	else \
		echo "	C: Error message printed as follows:"; \
		cat STDERR; \
	fi
test4.2.3:
	if [[ ! -s STDOUT ]]; \
	then \
		echo "	C: No output."; \
	else \
		echo "	W: Should not be any output."; \
	fi

test5.1.1:
	echo "Test 5.1: Testing with both input and output files given..."; \
	./lab0 --output="STDOUT" --input="STDIN" 2>STDERR; \
	if [[ $$? -eq 0 ]]; \
	then \
		echo "	C: Correct Exit Code."; \
	else \
		echo "	W: Wrong Exit Code."; \
	fi
test5.1.2:
	cmp STDIN STDOUT > /dev/null; \
	if [[ $$? -eq 0 ]]; \
	then \
		echo "	C: Output matches input."; \
	else \
		echo "	W: Output does not match with input."; \
	fi
test5.1.3:
	if [[ ! -s STDERR ]]; \
	then \
		echo "	C: No error messsage."; \
	else \
		echo "	W: There should not be any error message."; \
	fi

test6.1:
	echo "Test 6: Testing with segfault..."; \
	./lab0 --segfault; \
	if [[ $$? -eq 139 ]]; \
	then \
		echo "	C: Correct Exit Code."; \
	else \
		echo "	W: Wrong Exit Code."; \
	fi


test7.1.1:
	echo "Test 7.1: Testing with segfault and catch..."; \
	./lab0 --segfault --catch 2>STDERR; \
	if [[ $$? -eq 4 ]]; \
	then \
		echo "	C: Correct Exit Code."; \
	else \
		echo "	W: Wrong Exit Code."; \
	fi
test7.1.2:
	if [[ ! -s STDERR ]]; \
	then \
		echo "	W: No error messsage."; \
	else \
		echo "	C: Error message printed as follows:"; \
		cat STDERR; \
	fi
