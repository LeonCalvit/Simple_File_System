CC=gcc
CFLAGS=-Wall


initfs: formatfs.c softwaredisk.c
	gcc -o initfs.out formatfs.c softwaredisk.c -I.

test1: filesystem.c test1.c softwaredisk.c
	gcc -o test1.out softwaredisk.c filesystem.c test1.c -I.

softwaredisktest: exercisesoftwaredisk.c softwaredisk.c
	gcc -o softwaredisktest.out exercisesoftwaredisk.c softwaredisk.c

clean:
	rm *.out