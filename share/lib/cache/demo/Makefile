
ifndef MAIN
  MAIN = demo
endif

###############################################################################

PNETC_OPTS += -I$(SDPA_HOME)/share/sdpa/xml

PUT_PORT += num_slots=4L
PUT_PORT += num_id=6L
PUT_PORT += multiplicity=4L

NOT_STARTS_WITH += init_cache
NOT_STARTS_WITH += multiply
NOT_STARTS_WITH += generate_id
NOT_STARTS_WITH += get_slot_for_id
NOT_STARTS_WITH += done_with_slot

###############################################################################

DEP += Makefile

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
