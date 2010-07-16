include make.defines

all:
	$(MAKE) -C src 

testmods:
	$(MAKE) -C tests

clean:
	rm -rf bin/* lib/*.so *~ \#_*
	$(MAKE) -C src clean
	$(MAKE) -C tests clean


runfvm:
	cp bin/fvm_standalone /opt/cluster/PV-4D.pspro/bin/rui/
	 /opt/cluster/PV-4D.pspro/bin/rui/fvm_standalone &
