SUBDIRS = diff shrink op2 stack

ifeq ($(DESTDIR),)
DESTDIR=$(SDPA_HOME)/libexec/apps/parsu
endif

export DESTDIR

ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS=all
endif

.PHONY : all $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	make -C $@ $(MAKECMDGOALS)

.DEFAULT:
	for d in $(SUBDIRS) ; do \
	  make -C $$d $(MAKECMDGOALS) ; \
	done
