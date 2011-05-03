SUBDIRS = diff shrink op2 stack

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
