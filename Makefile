# now using Makefile for shortcut

build/ucc:
	mkdir -p build && cd build && cmake .. && cmake --build .

test: build/ucc
	./test.sh

clean:
	rm -fr build *.o *~ tmp*

.PHONY: build/ucc test clean
