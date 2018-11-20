CC=gcc
CFLAGS=-Wall
DEPS = filesystem.c softwaredisk.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

initfs: formatfs.c
	$(CC) $(CFLAGS)

test1 : test1.c
	$(CC) -o $@ $^ $(CFLAGS)

test2 : test2.c
	$(CC) -o $@ $^ $(CFLAGS)


clean:
	rm *.o prog3