#*********************************************************
# Makefile to build Z0remig3d 
#
#
#  module load  compiler/intel   
#
#*********************************************************

#------------- FFTW lib -------------------
FFTW3_INCL_DIR=/usr/include
FFTW3_LIB_DIR=/usr/lib64

#------------- C++ compiler
CXX=/opt/cluster/Intel/cce/9.1.047/bin/icpc


DEFINES = -Dpc_test_EXPORTS -DSHMEM -DDEBUGALLOC -DDEBUGCOMM -DSHMEM -DUSE_STL_TR1 -DNDEBUG -g -ggdb
INCLUDES =-I/p/herc/itwm/hpc/soft/fhglog/1.2.0/intel/include
INCLUDES+=-I/p/herc/itwm/hpc/soft/boost/1.38/intel/include/boost-1_38
INCLUDES+=-I/p/herc/itwm/hpc/soft/sdpa/fvm-pc/include
INCLUDES+=-I/p/herc/itwm/hpc/ap/SDPA/sdpa/trunk
INCLUDES+=-I/p/herc/itwm/hpc/ap/SDPA/nre-worker/trunk
INCLUDES+=-I/opt/cluster/PV-4D.pspro/include
INCLUDES+=-I . -I${FFTW3_INCL_DIR}

#================ CXXFLAGS
CXXFLAGS =  $(DEFINES) -wd383 -wd981 -O3 -fPIC $(INCLUDES)

#--------- LNKR -------
LNKR=${CXX}

#================ LIBFLAGS
LIBFLAGS= -fPIC -wd383 -wd981 -O3 -DNDEBUG -L${FFTW3_LIB_DIR} -L/opt/cluster/PV-4D.pspro/lib64 -Wl,-rpath=/opt/cluster/PV-4D.pspro/lib64:/opt/cluster/Intel/cce/9.1.047/lib -L.

#=================== LIBS
CXXLIBS=${LIBFLAGS} -lm -lc -lfftw3f -lguide -lpthread -libverbs12 -lPv4dVM4

#========= make all ==============
#all:libreInit.so libreNdlCfg.so libreProp.so libreUpdt.so libreTst.so
all:libreInit.so   libreTst.so

#all:libreMig.so

#---------------- link fe -----------
#libreMig.so: utls.o reMig.o
#	${LNKR}  -o libreMig.so $(CXXLIBS) \
#	-shared -Wl,-soname,libreMig.so reMig.o utls.o


# == short explanation  <sdpa-transition> - <obj. files>: ==
# "init" : pfafft.o reApplInit.o 
# "read input": utls.o reReadInp.o 
# "read velocity": reReadVel.o
# "calc one level": fft.o  propagatorsOpt.o  utilsOpt.o calcOneLevl.o 
# "update": reUpdt.o
# "prefix sum": rePrefixSum.o
#
# all of the transition modules linked (just for testing them) in libreInit.so

libreInit.so: cfg.o cfg-test.o  pfafft.o reApplInit.o  utls.o reReadInp.o reReadVel.o \
	calcOneLevl.o reUpdt.o
	${LNKR}  -o libreInit.so $(CXXLIBS) \
	-shared -Wl,-soname,libreInit.so cfg-test.o cfg.o \
	pfafft.o reApplInit.o \
	utls.o reReadInp.o reReadVel.o \
	fft.o  propagatorsOpt.o  utilsOpt.o calcOneLevl.o \
	reUpdt.o

#libreProp.so: reProp.o
#	${LNKR}  -o libreProp.so $(CXXLIBS) \
#	-shared -Wl,-soname,libreProp.so reProp.o

#libreUpdt.so: reUpdt.o
#	${LNKR}  -o libreUpdt.so $(CXXLIBS) \
#	-shared -Wl,-soname,libreUpdt.so reUpdt.o

libreTst.so: reTst.o
	${LNKR}  -o libreTst.so $(CXXLIBS) \
	-shared -Wl,-soname,libreTst.so reTst.o 


#libpc-test.so: pc-test.cpp
#        /opt/cluster/Intel/cce/9.1.047/bin/icpc $(DEFINES) -wd383 -wd981 -O3 -fPIC $(INCLUDES) -o pc-test.cpp.o -c pc-test.cpp
#        /opt/cluster/Intel/cce/9.1.047/bin/icpc  -fPIC -wd383 -wd981 -O3 -DNDEBUG  -shared -Wl,-soname,libpc-test.so -o libpc-test.so pc-test.cpp.o


#--------------- the modules -------
utls.o: reGlbStructs.h utls.h utls.cpp
	$(CXX) $(CXXFLAGS) -c utls.cpp

cfg.o: reGlbStructs.h cfg.h cfg.cpp
	$(CXX) $(CXXFLAGS) -c cfg.cpp

cfg-test.o: reGlbStructs.h cfg.h reApplInit.h reReadInp.h reReadVel.h \
	calcOneLevl.h reUpdt.h cfg-test.cpp
	$(CXX) $(CXXFLAGS) -c cfg-test.cpp

pfafft.o: cwp.h pfafft.h pfafft.cpp
	$(CXX) $(CXXFLAGS) -c pfafft.cpp

reApplInit.o: reGlbStructs.h pfafft.h reApplInit.h reApplInit.cpp
	$(CXX) $(CXXFLAGS) -c reApplInit.cpp

reReadInp.o: reGlbStructs.h utls.h reReadInp.h reReadInp.cpp
	$(CXX) $(CXXFLAGS) -c reReadInp.cpp

reReadVel.o: reGlbStructs.h utls.h reReadVel.h reReadVel.cpp
	$(CXX) $(CXXFLAGS) -c reReadVel.cpp

calcOneLevl.o: reGlbStructs.h utls.h propagatorsOpt.h utilsOpt.h calcOneLevl.cpp
	$(CXX) $(CXXFLAGS) -c calcOneLevl.cpp

#reProp.o: reGlbStructs.h reProp.h reProp.cpp
#	$(CXX) $(CXXFLAGS) -c reProp.cpp

reUpdt.o: reGlbStructs.h reUpdt.h reUpdt.cpp
	$(CXX) $(CXXFLAGS) -c reUpdt.cpp

reTst.o: reGlbStructs.h reTst.h reTst.cpp
	$(CXX) $(CXXFLAGS) -c reTst.cpp

#=============== clean ==================			 
clean:
	rm -f utls.o cfg.o cfg-test.o pfafft.o reApplInit.o reReadInp.o reReadVel.o calcOneLevl.o reProp.o reUpdt.o reTst.o *.so 
