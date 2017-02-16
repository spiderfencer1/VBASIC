CFLAGS=std=c99
OUT=basicc

all:
	gcc -$(CFLAGS) *.c -o $(OUT)

test:
	$(MAKE) all
	clear
	./$(OUT) test.bas
