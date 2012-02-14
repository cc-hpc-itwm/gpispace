include(pnet_macros)

pnet_compile(pnetv-asian_wrapper wrapper.xml
  OUTPUT asian.pnet
  FLAGS --no-inline=1
)
get_target_property(PNET_FILE pnet-pnetv-asian_wrapper LOCATION)
add_test(NAME test_pnetv_use_sequence COMMAND pnetv ${PNET_FILE})
