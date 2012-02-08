include(pnet_macros)

pnet_compile(pnetv-kdm_full kdm.xml
  OUTPUT kdm_full.pnet
  FLAGS --no-inline=1
  INCLUDES .
)
get_target_property(PNET_FILE pnet-pnetv-kdm_full LOCATION)
add_test(NAME test_pnetv_kdm_full COMMAND pnetv ${PNET_FILE})
