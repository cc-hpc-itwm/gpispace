ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

MAIN = tracebytrace

CXXINCLUDEPATHS += ../../util
CXXINCLUDEPATHS += $(SDPA_HOME)/include/seis
CXXINCLUDEPATHS += $(SDPA_HOME)/include/process

CXXLIBRARYPATHS += $(SDPA_LIBEXEC)/sdpa


NOT_ENDS_WITH += generate
NOT_ENDS_WITH += generate_slot
NOT_ENDS_WITH += generate_package

NOT_STARTS_WITH += dup
NOT_STARTS_WITH += scatter
NOT_STARTS_WITH += eat
NOT_STARTS_WITH += wait

include $(SDPA_HOME)/share/sdpa/make/common.mk
