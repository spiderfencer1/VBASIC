CFLAGS=std=c99
OUT=basicc

all:
	gcc -$(CFLAGS) -o $(OUT) src/*.c

clean:
	rm -rf $(OUT)

pre:
	gcc -$(CFLAGS) -E src/*.c 

test:
	$(MAKE) all
	clear
	./$(OUT) test.bas
