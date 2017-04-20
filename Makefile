CC=gcc
CFLAGS=std=c99 -o
OUT=basicc

all:
	$(CC) src/*.c -$(CFLAGS) $(OUT)

clean:
	/bin/rm -rf a.* *.o basicc

test:
	$(MAKE) clean
	$(MAKE)
	./basicc basic/a.bas >> a.s
	./casm a
