CC=gcc
CFLAGS=-g
LDFLAGS=-pthread -lm

all: master bin_adder

clock.o: clock.c clock.h
	$(CC) $(CFLAGS) -c clock.c

shm.o: shm.c shm.h
	$(CC) $(CFLAGS) -c shm.c

master: main.c clock.o shm.o
	$(CC) $(CFLAGS) main.c clock.o shm.o -o master $(LDFLAGS)

bin_adder: bin_adder.c clock.o shm.o
	$(CC) $(CFLAGS) bin_adder.c clock.o shm.o -o bin_adder $(LDFLAGS)

clean:
	rm -f master bin_adder *.o adder_log.txt
