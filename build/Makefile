all: mndd mnde mndb

clean:
	rm -vf *.o mndd mnde mndb

install: mndd mnde mndb
	mkdir -p /usr/local/bin
	cp mndd /usr/local/bin/
	cp mnde /usr/local/bin/
	cp mndb /usr/local/bin/

mndd: mndd.o
	cc -o mndd mndd.o -L/usr/local/lib/ -lmnd -lm

mnde: mnde.o mndc.o
	cc -o mnde mnde.o mndc.o -L/usr/local/lib/ -lmnd -lm

mndb: mndb.o mndc.o
	cc -o mndb mndb.o mndc.o -L/usr/local/lib/ -lmnd -lm

mndd.o: ../src/mndd.c
	cc -c -std=gnu11 ../src/mndd.c

mnde.o: ../src/mnde.c
	cc -c -std=gnu11 ../src/mnde.c

mndb.o: ../src/mndb.c
	cc -c -std=gnu11 ../src/mndb.c

mndc.o: ../src/mndc.c
	cc -c -std=gnu11 ../src/mndc.c
