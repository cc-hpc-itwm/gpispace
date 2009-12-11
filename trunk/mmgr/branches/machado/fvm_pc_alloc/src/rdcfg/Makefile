CC = gcc -Wall -I.

%.c: %.y
%.c: %.l
%.tab.c %.tab.h: %.y
	bison --defines $<
%.c: %.l %.tab.h
	flex -o $@ $<
%: %.tab.o %.o
	$(CC) -o $@ $^
main: rdcfg.tab.o rdcfg.o main.o
	$(CC) -o $@ $^
	rm -f rdcfg.tab.h
	rm -f $^