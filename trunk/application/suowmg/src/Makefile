# Makefile for inversionMH

CC=icc
OPT  = -O3 -axSSE4.2,SSE4.1,SSSE3
FLAGS = -Wall -wd981 -wd1419 -wd1418
C99  = -std=c99
LINKS= -lsu -lpar -lcwp -lfftw3f -lm
#INC= -I/prog/sdpsoft/SU/include 
#LIB= -L/prog/sdpsoft/SU/lib
INC = -I/p/herc/itwm/hpc/mr/cwp/include -I/p/herc/itwm/hpc/mr/fftw-3.2.2/include
LIB = -L/p/herc/itwm/hpc/mr/cwp/lib -L/p/herc/itwm/hpc/mr/fftw-3.2.2/lib
SUBS= 	./suowmg.o	\
	./owmg.o	\
	./propagators.o	\
	./mods.o	\
	./tools.o	\
	./rtc.o		\
	./pfafft.o

all: suowmg

suowmg: $(SUBS)
	$(CC) $(FLAGS) $(OPT) $(C99) $(LIB) $(SUBS) -o suowmg ${LINKS}

rtc.o: rtc.c
	$(CC) $(OPT)  $(INC) -c rtc.c

.SUFFIXES: .o .c 
.c.o:
	$(CC) $(FLAGS) $(OPT) $(C99) $(INC) -c $*.c

todo:
	grep -in todo *.c *.h

clean:
	\rm -f *.o core

# Explicit dependencies
suowmg.o: owmg.h tools.h mods.h
owmg.o: owmg.h tools.h propagators.h pfafft.h rtc.h
tools.o: tools.h
propagators.o: propagators.h tools.h
rtc.o: rtc.h
tools.o: tools.h
