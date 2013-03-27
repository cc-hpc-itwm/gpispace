JAVA_HOME = /usr/lib/jvm/java-8-oracle
INCLUDE += -I $(JAVA_HOME)/include
INCLUDE += -I $(JAVA_HOME)/include/linux
INCLUDE += -I /home/alex/.local/sdpa/include
CFLAGS += -fPIC $(INCLUDE)
CXXFLAGS += $(CFLAGS)
LDFLAGS += -L /home/alex/.local/sdpa/lib -L /usr/local/lib
LDFLAGS += -lfhg-util -lwe-dev -lgpi-space-pc-client -lgpi-space-pc-segment -lfhg-plugin-core -lfhglog -lboost_filesystem -lboost_serialization -lboost_thread -lboost_system

default: build

all: build run
build: main.class

%.i %_wrap.c: Makefile
	swig -java $*.i

libgpi.so: gpi_wrap.o gpi.o
	$(CXX) -shared $^ $(LDFLAGS) -o $@

%.class %.java:
	javac $*.java

main.class: libgpi.so

run: main.class
	@java main

clean:
	rm -f *.class
	rm -f *.o
	rm -f *.so
	rm -f *JNI.java
	rm -f SWIGTYPE*
	rm -f *_wrap.c
	rm -f *~
