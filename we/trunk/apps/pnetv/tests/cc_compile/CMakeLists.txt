include(pnet_macros)

pnet_compile(pnetv-cc_compile compile.xml
  OUTPUT cc_compile.pnet
  FLAGS --no-inline=1
)
get_target_property(PNET_FILE pnet-pnetv-cc_compile LOCATION)
add_test(NAME test_pnetv_cc_compile COMMAND pnetv ${PNET_FILE})
