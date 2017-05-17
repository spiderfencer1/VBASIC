CC=gcc
CLAGS=std=c99 -o
OUT=basicc

all:
	$(CC) src/*.c -$(CFLAGS) $(OUT)

clean:
	/bin/rm -rf a.* *.o basicc

test:
	$(MAKE) clean
	$(MAKE)
	./basicc tests/basic/b.bas > b.s
	./casm b
