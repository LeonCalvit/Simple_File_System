CC=gcc
CFLAGS=-Wall
DEPS = filesystem.c softwaredisk.c

initfs: formatfs.c softwaredisk.c
	gcc -o initfs.out formatfs.c softwaredisk.c -I.

test1: filesystem.c test1.c
	gcc -o test1.out filesystem.c test1.c -I.

softwaredisktest: exercisesoftwaredisk.c softwaredisk.c
	gcc -o softwaredisktest.out exercisesoftwaredisk.c softwaredisk.c

clean:
	rm *.out