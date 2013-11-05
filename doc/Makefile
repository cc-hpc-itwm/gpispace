# mirko.rahn@itwm.fhg.de

ifndef BUILD_ROOT
  $(error Variable BUILD_ROOT is empty)
endif

THISDIR       := $(notdir $(CURDIR))
BUILDDIR      := $(BUILD_ROOT)/$(THISDIR)
CONFIGURATION := $(BUILDDIR)/Makefile.configuration

ifneq "$(wildcard $(CONFIGURATION))" ""
  include $(CONFIGURATION)
endif

ifndef $(ASCIIDOC_EXECUTABLE)
  ASCIIDOC_EXECUTABLE := $(shell which asciidoc)
endif

ifeq "$(ASCIIDOC_EXECUTABLE)" ""
  $(error Variable ASCIIDOC_EXECUTABLE is empty)
endif

ifndef ASCIIDOC_VERSION
  ASCIIDOC_VERSION       := $(word 2,$(shell $(ASCIIDOC_EXECUTABLE) --version))
  ASCIIDOC_VERSION_MAJOR := $(word 1,$(subst ., ,$(ASCIIDOC_VERSION)))
  ASCIIDOC_VERSION_MINOR := $(word 2,$(subst ., ,$(ASCIIDOC_VERSION)))
  ASCIIDOC_VERSION_PATCH := $(word 3,$(subst ., ,$(ASCIIDOC_VERSION)))
  ASCIIDOC_VERSION_NUM   := $(shell echo $$((100 * $(ASCIIDOC_VERSION_MAJOR) + 10 * $(ASCIIDOC_VERSION_MINOR) + $(ASCIIDOC_VERSION_PATCH))))
  ASCIIDOC_VERSION_OKAY  := $(shell expr $(ASCIIDOC_VERSION_NUM) \>= 867)
endif

ifeq "$(ASCIIDOC_VERSION_OKAY)" "0"
  $(error Require asciidoc version at least 8.6.8, found $(ASCIIDOC_VERSION))
endif

ifndef ASCIIDOC_BINDIR
  ASCIIDOC_BINDIR := $(dir $(ASCIIDOC_EXECUTABLE))
endif

ifeq "$(ASCIIDOC_BINDIR)" ""
  $(error Variable ASCIIDOC_BINDIR is empty)
endif

ifndef A2X_EXECUTABLE
  A2X_EXECUTABLE := $(shell which a2x)
endif

ifeq "A2X_EXECUTABLE" ""
  $(error Variable A2X_EXECUTABLE is empty)
endif

ifndef A2X_VERSION
  A2X_VERSION       := $(word 2,$(shell $(A2X_EXECUTABLE) --version))
  A2X_VERSION_MAJOR := $(word 1,$(subst ., ,$(A2X_VERSION)))
  A2X_VERSION_MINOR := $(word 2,$(subst ., ,$(A2X_VERSION)))
  A2X_VERSION_PATCH := $(word 3,$(subst ., ,$(A2X_VERSION)))
  A2X_VERSION_NUM   := $(shell echo $$((100 * $(A2X_VERSION_MAJOR) + 10 * $(A2X_VERSION_MINOR) + $(A2X_VERSION_PATCH))))
  A2X_VERSION_OKAY  := $(shell expr $(A2X_VERSION_NUM) \>= 867)
endif

ifeq "$(A2X_VERSION_OKAY)" "0"
  $(error Require a2x version at least 8.6.8, found $(A2X_VERSION))
endif

ifndef SOURCE_HIGHLITE_EXECUTABLE
  SOURCE_HIGHLITE_EXECUTABLE := $(shell which source-highlight)
endif

ifeq "$(SOURCE_HIGHLITE_EXECUTABLE)" ""
  $(error Variable SOURCE_HIGHLITE_EXECUTABLE is empty)
endif

ifndef SOURCE_HIGHLITE_BINDIR
  SOURCE_HIGHLITE_BINDIR := $(dir $(SOURCE_HIGHLITE_EXECUTABLE))
endif

ifeq "$(SOURCE_HIGHLITE_BINDIR)" ""
  $(error Variable SOURCE_HIGHLITE_BINDIR is empty)
endif

###############################################################################

$(BUILDDIR):
	mkdir -p $@

$(BUILDDIR)/a2x:
	mkdir -p $@

###############################################################################

.PHONY: all

all: configuration.save        \
     $(BUILDDIR)/user.html     \
     $(BUILDDIR)/user.docbook  \
     $(BUILDDIR)/a2x/user.html \
     $(BUILDDIR)/a2x/user.pdf

###############################################################################

INPUT += user.txt

###############################################################################

ASCIIDOC = PATH=$(SOURCE_HIGHLITE_BINDIR):${PATH} $(ASCIIDOC_EXECUTABLE) -b $(1)

$(BUILDDIR)/user.html: | $(BUILDDIR)
$(BUILDDIR)/user.html: $(INPUT)
	$(call ASCIIDOC,xhtml11) -o $@ $<

$(BUILDDIR)/user.docbook: | $(BUILDDIR)
$(BUILDDIR)/user.docbook: $(INPUT)
	$(call ASCIIDOC,docbook) -o $@ $<

###############################################################################

A2X_OPTS += --no-xmllint
A2X_OPTS += --destination-dir=$(BUILDDIR)/a2x

A2X = PATH=$(ASCIIDOC_BINDIR):${PATH} $(A2X_EXECUTABLE) $(A2X_OPTS) -f $(1)

$(BUILDDIR)/a2x/user.html: | $(BUILDDIR)/a2x
$(BUILDDIR)/a2x/user.html: $(INPUT)
	$(call A2X,xhtml) $<

$(BUILDDIR)/a2x/user.pdf: | $(BUILDDIR)/a2x
$(BUILDDIR)/a2x/user.pdf: $(INPUT)
	$(call A2X,pdf) $<

###############################################################################

.PHONY: configuration.show configuration.save configuration.remove

configuration.show:
	@echo "BUILD_ROOT = $(BUILD_ROOT)"
	@echo "THISDIR = $(THISDIR)"
	@echo "BUILDDIR = $(BUILDDIR)"
	@echo "CONFIGURATION = $(CONFIGURATION)"
	@echo "ASCIIDOC_EXECUTABLE = $(ASCIIDOC_EXECUTABLE)"
	@echo "ASCIIDOC_VERSION = $(ASCIIDOC_VERSION)"
	@echo "ASCIIDOC_VERSION_MAJOR = $(ASCIIDOC_VERSION_MAJOR)"
	@echo "ASCIIDOC_VERSION_MINOR = $(ASCIIDOC_VERSION_MINOR)"
	@echo "ASCIIDOC_VERSION_PATCH = $(ASCIIDOC_VERSION_PATCH)"
	@echo "ASCIIDOC_VERSION_NUM = $(ASCIIDOC_VERSION_NUM)"
	@echo "ASCIIDOC_BINDIR = $(ASCIIDOC_BINDIR)"
	@echo "A2X_EXECUTABLE = $(A2X_EXECUTABLE)"
	@echo "A2X_VERSION = $(A2X_VERSION)"
	@echo "A2X_VERSION_MAJOR = $(A2X_VERSION_MAJOR)"
	@echo "A2X_VERSION_MINOR = $(A2X_VERSION_MINOR)"
	@echo "A2X_VERSION_PATCH = $(A2X_VERSION_PATCH)"
	@echo "A2X_VERSION_NUM = $(A2X_VERSION_NUM)"
	@echo "SOURCE_HIGHLITE_EXECUTABLE = $(SOURCE_HIGHLITE_EXECUTABLE)"
	@echo "SOURCE_HIGHLITE_BINDIR = $(SOURCE_HIGHLITE_BINDIR)"

$(CONFIGURATION): | $(BUILDDIR)
$(CONFIGURATION):
	$(MAKE) configuration.show > $(CONFIGURATION)

configuration.save: $(CONFIGURATION)

configuration.remove:
	$(RM) $(CONFIGURATION)
