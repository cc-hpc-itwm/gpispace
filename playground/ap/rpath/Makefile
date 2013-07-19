.PHONY: all clean libs bins

all: libs bins

libs:
	gcc -o lib/liba.so -shared -fpic -I lib/ lib/a.c
	gcc -o libb.so -Wl,-rpath,'$$ORIGIN/:$$ORIGIN/lib/' -shared -fpic -I lib/ -L lib/ b.c -la

bins:
	gcc -rdynamic -o dlopen dlopen.c -ldl

clean:
	rm -f dlopen
	rm -f libb.so
	rm -f lib/liba.so
