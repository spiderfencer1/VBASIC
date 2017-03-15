CFLAGS=std=c99
OUT=basicc

all:
	gcc -$(CFLAGS) -o $(OUT) src/*.c

clean:
	rm -rf $(OUT) a.s

c:
	$(MAKE) all
	clear
	./$(OUT) test.bas >> a.s

pre:
	gcc -$(CFLAGS) -E src/*.c 

test:
	$(MAKE) all
	clear
	./$(OUT) test.bas
