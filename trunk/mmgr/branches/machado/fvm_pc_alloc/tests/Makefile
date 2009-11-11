include ../make.defines

OBJ=test-allocation.o test-communication.o test-api.o test-dynamicArray.o test-dynamicArrayUsage.o test-dynamicArrayClean.o test-distribute.o
.cpp.o: $<
	$(CXX) $(CXXFLAGS) $(INCLUDE) -fPIC -g -c $< -o $@

tests: $(OBJ)
	$(CXX) $(CXXFLAGS) -shared -Wl,-soname,testAllocation-mod.so -o testAllocation-mod.so test-allocation.o $(OBJS)/Module.o
	$(CXX) $(CXXFLAGS) -shared -Wl,-soname,testCommunication-mod.so -o testCommunication-mod.so test-communication.o $(OBJS)/Module.o
	$(CXX) $(CXXFLAGS) -shared -Wl,-soname,testAPI-mod.so -o testAPI-mod.so test-api.o $(OBJS)/Module.o
	$(CXX) $(CXXFLAGS) -shared -Wl,-soname,testDynamicArrayInit-mod.so -o testDynamicArrayInit-mod.so test-dynamicArray.o $(OBJS)/Module.o
	$(CXX) $(CXXFLAGS) -shared -Wl,-soname,testDynamicArrayUsage-mod.so -o testDynamicArrayUsage-mod.so test-dynamicArrayUsage.o $(OBJS)/Module.o
	$(CXX) $(CXXFLAGS) -shared -Wl,-soname,testDynamicArrayClean-mod.so -o testDynamicArrayClean-mod.so test-dynamicArrayClean.o $(OBJS)/Module.o
	$(CXX) $(CXXFLAGS) -shared -Wl,-soname,testdistribute-mod.so -o testdistribute-mod.so test-distribute.o $(OBJS)/Module.o

clean:
	rm -rf *.o *~ *.a $(BIN) *.so *~ \#_*