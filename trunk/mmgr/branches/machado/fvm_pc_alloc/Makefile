include make.defines

all:
	make -C src 

testmods:
	make -C tests

clean:
	rm -rf bin/* lib/*.so *~ \#_*
	make -C src clean
	make -C tests clean


runfvm:
	cp bin/fvm_standalone /opt/cluster/PV-4D.pspro/bin/rui/
	 /opt/cluster/PV-4D.pspro/bin/rui/fvm_standalone &
