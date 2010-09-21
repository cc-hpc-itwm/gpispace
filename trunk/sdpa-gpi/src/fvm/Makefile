include ../../make.defines

OBJ=fvmAllocator.o fvmConfig.o fvmSync.o fvmLogger.o fvm.o fvm_standalone.o
all:fvm

.c.o:
	$(CC) $(CXXFLAGS) $(INCLUDE) -fPIC  -c $< -o $(OBJS)/$@

lib: $(OBJ)
	ar -rs libfvmcommon.a $(OBJS)/fvmAllocator.o $(OBJS)/fvm.o $(OBJS)/fvmLogger.o $(OBJS)/fvmConfig.o $(OBJS)/fvmSync.o $(OBJS)/stack.o $(OBJS)/list.o

fvm: fvm_standalone.o lib
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LPATH) -o $(BINDIR)/fvm_standalone $(OBJS)/fvm_standalone.o ./libfvmcommon.a $(TOPROOT)/ext/lib64/libmmgr.a $(LIBS)

clean:
	rm -rf *.o *~ *.a $(BIN) *.so *~ \#_*
