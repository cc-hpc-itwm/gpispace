SUBDIRS = build

ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS=all
endif

.PHONY : all $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.DEFAULT:
	for d in $(SUBDIRS) ; do \
	  $(MAKE) -C $$d $(MAKECMDGOALS) ; \
	done
