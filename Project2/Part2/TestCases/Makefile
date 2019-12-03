#Nathan Walzer - nwalzer

all: test1 test2

test1: test1.o
	gcc test1.o -o test1

test2: test2.o
	gcc test2.o -o test2

test1.o: test1.c
	gcc -g -c test1.c

test2.o: test2.c
	gcc -g -c test2.c

clean:
	rm -f *.o test1
	rm -f *.o test2
