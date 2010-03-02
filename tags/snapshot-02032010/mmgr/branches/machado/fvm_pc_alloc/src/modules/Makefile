include ../../make.defines

OBJ=Activity.o FallBackModuleLoader.o Module.o Parameter.o  Properties.o Token.o

all: $(OBJ)

.cpp.o: $(OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -fPIC  -c $< -o $(OBJS)/$@

clean:
	rm -rf *.o *~ *.a $(BIN) *.so *~ \#_*