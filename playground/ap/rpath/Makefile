.PHONY: all clean libs bins

all: libs bins

libs:
	mkdir -p lib libexec/plugins
	gcc -o lib/liba.so -shared -fpic -I lib/ a.c
	gcc -o libexec/plugins/libb.so -Wl,-rpath,'$$ORIGIN/:$$ORIGIN/lib/:$$ORIGIN/../lib/:$$ORIGIN/../../lib/' -shared -fpic -L lib/ b.c -la

bins:
	mkdir -p bin
	gcc -rdynamic -o bin/dlopen dlopen.c -ldl

clean:
	rm -rf bin
	rm -rf lib
	rm -rf libexec
