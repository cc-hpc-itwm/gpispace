MAIN = compile

###############################################################################

PUT_PORT += compiler="\"$(shell which g++)\""
PUT_PORT += cwd=\""$(PWD)"\"
# FIXME: PUT_PORT seems to add -p to all those arguments, dunno how to escape them correctly
PNETPUT_OPTS += -p "argv=List(\"-o\", \"$(BUILDDIR)/hello\", \"$(CURDIR)/src/hello.c\")"
CLEAN += clean/hello clean/sdpacc
BUILD += gen-sdpacc

###############################################################################

CXXLIBRARYPATHS += $(SDPA_HOME)/libexec/sdpa

###############################################################################

ifndef SDPA_HOME
$(error "SDPA_HOME is not set, please set it to the installation path of GPISpace")
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk

.PHONY: gen-sdpacc

gen-sdpacc: $(BUILDDIR)/sdpacc

$(BUILDDIR)/sdpacc: $(CURDIR)/bin/sdpacc.in $(NET)
	cat $^ > $@
	chmod +x $@

.PHONY: clean/hello
clean/hello:
	-$(RM) '$(BUILDDIR)/hello'

.PHONY: clean/sdpacc
clean/sdpacc:
	-$(RM) '$(BUILDDIR)/sdpacc'
