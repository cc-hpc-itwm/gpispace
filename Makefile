SUBDIRS = build

ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS=all
endif

.PHONY : all config release $(SUBDIRS)

all: $(SUBDIRS)

config:
	@ccmake build

release:
	cd build && cpack --config cpack-rel.cmake

incr-build:
	@./incr-build-count

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.DEFAULT:
	for d in $(SUBDIRS) ; do \
	  $(MAKE) -C $$d $(MAKECMDGOALS) ; \
	done
