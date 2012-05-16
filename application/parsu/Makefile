SUBDIRS = diff tracebytrace op2 stack shrink

ifeq ($(DESTDIR),)
DESTDIR=$(SDPA_HOME)/libexec/sdpa/apps/parsu
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
