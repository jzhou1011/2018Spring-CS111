#!/bin/bash


# expected return codes
EXIT_OK=0
EXIT_ARG=1

echo "Test 1: Testing unrecognized arguments..."
./lab4b --wrongarg >STDOUT 2> STDERR
if [ $? -eq $EXIT_ARG ]
then
	echo "	C: Correct Exit Code."
else
	echo "	W: Wrong Exit Code."
fi
if [ ! -s STDERR ]
then
	echo "	W: Should output error message when wrong arguments are used."
else
	echo "	C: Error message and Usage message shoule be outputted as follows:"
	cat STDERR
fi
if [ ! -s STDOUT ]	#if output empty
then
	echo "	C: No output."
else
	echo "	W: Should not be any output."
fi

echo "Test 2: Testing lab4b with standard arguments..."
echo "SCALE=F" >> STDIN
echo "PERIOD=5" >> STDIN
echo "OFF" >> STDIN
./lab4b --period=3 --scale=F --log=LOGFILE < STDIN > STDOUT 2>STDERR ; \
if [ $? -eq $EXIT_OK ]
then
	echo "	C: Correct Exit Code."
else
	echo "	W: Wrong Exit Code."
fi
if [ ! -s LOGFILE ]
then
	echo "	W: No log."
else
	echo "	C: Log file contents are as follows:"
	cat LOGFILE
fi
if [ ! -s STDERR ]
then
	echo "	C: No error messsage."
else
	echo "	W: There should not be any error message."
fi

