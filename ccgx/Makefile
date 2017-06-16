all: mndd mnde

clean:
	rm -vf *.o mndd mnde

mndd: mndd.o
	$$CC -o mndd mndd.o -L../../mndlib/ccgx/ -lmnd -lm

mnde: mnde.o
	$$CC -o mnde mnde.o -L../../mndlib/ccgx -lmnd -lm

mndd.o: ../src/mndd.c
	$$CC -c -std=gnu11 ../src/mndd.c

mnde.o: ../src/mnde.c
	$$CC -c -std=gnu11 ../src/mnde.c
