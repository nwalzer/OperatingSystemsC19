#Nathan Walzer - nwalzer

all: Sys1Test

Sys1Test: Sys1Test.o
	gcc Sys1Test.o -o Sys1Test

Sys1Test.o: Sys1Test.c
	gcc -g -c Sys1Test.c

clean:
	rm -f *.o Sys1Test
