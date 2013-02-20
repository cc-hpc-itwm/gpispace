default: all

all: guard-failure guard-success

guard-success: guard.c guard.h req-guard.c req-guard.h
	@echo "Compiling/Linking matching objects"
	@$(CC) -fPIC -c -o guard.o guard.c
	@$(CC) -fPIC -c -o req-guard.o req-guard.c
	@$(CC) main.c req-guard.o guard.o
	@./a.out

# expected to fail
guard-failure: guard.c guard.h req-guard.c req-guard.h
	@echo "Compiling/Linking non-matching objects"
	@$(CC) -fPIC -c -o guard.o guard.c
	@$(CC) -fPIC -DSYMBOL_VALUE=foo -c -o req-guard.o req-guard.c
	-@$(CC) main.c req-guard.o guard.o

clean:
	rm -f *.o
	rm -f a.out
