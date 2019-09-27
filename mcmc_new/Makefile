SHELL = /bin/sh
CFLAGS = -O2 -DNDEBUG
VERSION = `grep 'define SLL_VERSION_STRING' main.cpp | sed 's/.*"\(.*\)"/\1/'`
FILES   = $(wildcard *.hpp)

main: main.cpp $(FILES)
	g++ $(CFLAGS) main.cpp  -o main -std=c++11 -lboost_program_options

clean:
	rm -f sll SLL-*.tar.gz gmon.out

recompile: clean main

package: clean
	tar -cpzf SLL-$(VERSION).tar.gz sll.cpp *.hpp Makefile README example.dat

