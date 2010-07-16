CC = gcc -Wall -I. -D_GNU_SOURCE

%.c: %.y
%.c: %.l
%.tab.c %.tab.h: %.y
	bison --defines $<
%.c: %.l %.tab.h
	flex -o $@ $<
%: %.tab.o %.o
	$(CC) -o $@ $^
main: fvmConfig.tab.o fvmConfig.o main.o
	$(CC) -o $@ $^
	rm -f fvmConfig.tab.h
	rm -f $^