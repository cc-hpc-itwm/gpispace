include(pnet_macros)

pnet_compile(pnetv-use_sequence use_sequence.xml
  OUTPUT use_sequence.pnet
  FLAGS --no-inline=1
)
get_target_property(PNET_FILE pnet-pnetv-use_sequence LOCATION)
add_test(NAME test_pnetv_use_sequence COMMAND pnetv ${PNET_FILE})
