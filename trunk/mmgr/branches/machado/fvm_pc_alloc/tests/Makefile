include ../make.defines

tests:
	$(CXX) $(CXXFLAGS) $(INCLUDE) -fPIC -c test-allocation.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -fPIC -c test-communication.cpp
	$(CXX) $(CXXFLAGS) -shared -Wl,-soname,testAllocation-mod.so -o testAllocation-mod.so test-allocation.o $(OBJS)/Module.o
	$(CXX) $(CXXFLAGS) -shared -Wl,-soname,testCommunication-mod.so -o testCommunication-mod.so test-communication.o $(OBJS)/Module.o

clean:
	rm -rf *.o *~ *.a $(BIN) *.so *~ \#_*