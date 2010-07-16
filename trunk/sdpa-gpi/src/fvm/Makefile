include ../../make.defines

OBJ=fvmAllocator.o fvmConfig.o fvmSync.o fvmLogger.o fvm.o fvm_standalone.o
all:fvm 

.c.o:
	$(CXX) $(CXXFLAGS) $(INCLUDE) -fPIC  -c $< -o $(OBJS)/$@

lib: $(OBJ)
	ar -rs libfvmcommon.a $(OBJS)/fvmAllocator.o $(OBJS)/fvm.o $(OBJS)/fvmLogger.o $(OBJS)/fvmConfig.o $(OBJS)/fvmSync.o $(OBJS)/list.o $(OBJS)/stack.o

fvm: fvm_standalone.o lib
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LPATH) -o $(BINDIR)/fvm_standalone $(OBJS)/fvm_standalone.o libfvmcommon.a $(TOPROOT)/lib/libmmgr.a $(LIBS)

clean:
	rm -rf *.o *~ *.a $(BIN) *.so *~ \#_*