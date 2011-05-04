SUBDIRS = parsu suowmg asian/pnet example/pi
TBZ = applications.tar.bz2

ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS=all
endif

.PHONY : all $(SUBDIRS) package

all: $(SUBDIRS)

$(SUBDIRS):
	make -C $@ $(MAKECMDGOALS)

.DEFAULT:
	for d in $(SUBDIRS) ; do \
	  make -C $$d $(MAKECMDGOALS) ; \
	done

package: $(TBZ)

$(TBZ):
	tar cfjv $@ Makefile $(SUBDIRS)
