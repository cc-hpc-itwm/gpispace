
MAIN = pi

###############################################################################

NOT_STARTS_WITH += generate
NOT_STARTS_WITH += get_key
NOT_STARTS_WITH += run

###############################################################################

PUT_PORT += num_packet=500L
PUT_PORT += points_per_packet=1000000L
PUT_PORT += credit_generate=20L
PUT_PORT += credit_run=10L
PUT_PORT += credit_get_key=20L
PUT_PORT += seed=3141L

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk

WE_EXEC_WORKER=12
