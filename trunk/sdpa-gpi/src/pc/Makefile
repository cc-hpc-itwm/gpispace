include ../../make.defines

OBJ=pc-mod.o pc.o

all: pclib pc

.cpp.o: $<
	$(CXX) $(CXXFLAGS) $(INCLUDE) -fPIC  -c $< -o $(OBJS)/$@

.c.o: $<
	$(CXX) $(CXXFLAGS) $(INCLUDE) -fPIC  -c $< -o $(OBJS)/$@

pclib: $(OBJ) 
	$(CXX) $(CXXFLAGS) -shared -Wl,-soname,libpc-mod.so -o $(LIBSDIR)/libpc-mod.so $(OBJS)/pc-mod.o $(OBJS)/pc.o $(OBJS)/fvmConfig.o $(OBJS)/Activity.o $(OBJS)/FallBackModuleLoader.o $(OBJS)/Module.o $(OBJS)/Parameter.o  $(OBJS)/Properties.o $(OBJS)/Token.o 

pc: pc_standalone.o $(OBJS)/Activity.o $(OBJS)/FallBackModuleLoader.o $(OBJS)/Module.o $(OBJS)/Parameter.o  $(OBJS)/Properties.o $(OBJS)/Token.o
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LPATH)  -o $(BINDIR)/pc_standalone $(OBJS)/pc_standalone.o $(OBJS)/Activity.o $(OBJS)/FallBackModuleLoader.o $(OBJS)/Module.o $(OBJS)/Parameter.o  $(OBJS)/Properties.o $(OBJS)/Token.o  $(LIBS)

clean:
	rm -rf *.o *~ *.a $(BIN) *.so *~ \#_*