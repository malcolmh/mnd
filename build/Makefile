all: mndd mnde

clean:
	rm -vf *.o mndd mnde

install:
	cp mndd /usr/local/bin/
	cp mnde /usr/local/bin/

mndd: mndd.o
	cc -o mndd mndd.o -L/usr/local/lib/ -lmnd -lm

mnde: mnde.o
	cc -o mnde mnde.o -L/usr/local/lib/ -lmnd -lm

mndd.o: ../src/mndd.c ../src/n0183.h ../src/n2000.h ../src/m1371.h
	cc -c -std=gnu11 ../src/mndd.c

mnde.o: ../src/mnde.c ../src/e2000.h ../src/n2000.h ../src/m1371.h
	cc -c -std=gnu11 ../src/mnde.c
