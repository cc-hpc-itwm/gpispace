include ../../make.defines

OBJ= list.c stack.c

.c.o: $(OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -fPIC  -c $< -o $(OBJS)/$@


all: list.o stack.o


clean:
	rm -rf *.o *~ *.a $(BIN) *.so *~ \#_*