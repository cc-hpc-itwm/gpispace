
ifndef MAIN
  MAIN = get
endif

###############################################################################

PNETC_OPTS += -I$(SDPA_HOME)/share/sdpa/xml

#PNETPUT_OPTS += -p cache='[by_id:=[slot:={|21->4,12->7,13->9|}],by_slot:=[id:={|4->21,7->12,9->13|},num_users:={|4->1,7->0,9->0|}],slots:=[available:={: 7 9:},empty:={: 5 6 8:},filled:={: 7 9:}]]'
#PUT_PORT += id=12
#PUT_PORT += id=21
#PUT_PORT += id=41

PNETPUT_OPTS += -p cache='[by_id:=[slot:={|21->4,12->7,13->9|}],by_slot:=[id:={|4->21,7->12,9->13|},num_users:={|4->1,7->0,9->0|}],slots:=[available:={: 7 9:},empty:={::},filled:={: 7 9:}]]'
PUT_PORT += id=41


###############################################################################

DEP += Makefile

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
