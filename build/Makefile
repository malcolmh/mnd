all: mndd mnde

clean:
	rm -vf *.o mndd mnde

install: mndd mnde
	mkdir -p /usr/local/bin
	cp mndd /usr/local/bin/
	cp mnde /usr/local/bin/

mndd: mndd.o wavinit.o
	cc -o mndd mndd.o wavinit.o -L/usr/local/lib/ -lmnd -lm

mnde: mnde.o
	cc -o mnde mnde.o -L/usr/local/lib/ -lmnd -lm

mndd.o: ../src/mndd.c ../src/wavinit.c
	cc -c -std=gnu11 ../src/mndd.c ../src/wavinit.c

mnde.o: ../src/mnde.c
	cc -c -std=gnu11 ../src/mnde.c
