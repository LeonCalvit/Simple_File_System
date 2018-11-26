CC=gcc
CFLAGS=-Wall
DEPS= filesystem.h softwaredisk.h



%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

initfs: formatfs.c softwaredisk.c
	gcc -o initfs.out formatfs.c softwaredisk.c -I.

test1: filesystem.o softwaredisk.o test1.o
	$(CC) -o $@.out $^ $(CFLAGS)

softwaredisktest: exercisesoftwaredisk.c softwaredisk.c
	gcc -o softwaredisktest.out exercisesoftwaredisk.c softwaredisk.c

clean:
	rm *.out