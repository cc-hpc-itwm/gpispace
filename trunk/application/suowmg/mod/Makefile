# Makefile for inversionMH

CC=icc

OPT  = -O3 -axSSE4.2,SSE4.1,SSSE3 
#OPT = -g -ggdb
FLAGS = -Wall -wd981 -wd1419 -wd1418
FLAGS += $(addprefix -D,$(DEFS))

C99  = -std=c99

LINK += su
LINK += par
LINK += cwp
LINK += fftw3f
LINK += m
LINK += MCTP1
LINK += rt
LINKS= $(addprefix -l,$(LINK))

#INC= -I/prog/sdpsoft/SU/include
#LIB= -L/prog/sdpsoft/SU/lib

INCS += /p/herc/itwm/hpc/mr/cwp/include
INCS += /p/herc/itwm/hpc/mr/fftw-3.2.2/include
INCS += /opt/cluster/GPI/include
INCS += /p/herc/itwm/hpc/mr/sdpa/trunk/fvm-pc/trunk
INC = $(addprefix -I,$(INCS))

LIBS += /p/herc/itwm/hpc/mr/cwp/lib
LIBS += /p/herc/itwm/hpc/mr/fftw-3.2.2/lib
LIBS += /opt/cluster/GPI/lib64
LIB = $(addprefix -L,$(LIBS))

SUBS= 	./suowmg.o	\
	./owmg.o	\
	./propagators.o	\
	./mods.o	\
	./tools.o	\
	./rtc.o		\
	./pfafft.o
	
all: suowmg fread.o libfread.so

suowmg: $(SUBS)
	$(CC) $(FLAGS) $(OPT) $(C99) $(LIB) $(SUBS) -o suowmg ${LINKS}

fread.o: fread.c fread.h
	$(CC) $(FLAGS) $(OPT) $(C99) $(LIB) $(INC) -c $< -o $@ -fPIC
	
libfread.so: fread.o
	$(CC) -shared -o $@ $^
	
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
