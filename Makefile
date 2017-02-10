CFLAGS=std=c99
OUT=basicc

all:
	gcc -$(CFLAGS) *.c -o $(OUT)
