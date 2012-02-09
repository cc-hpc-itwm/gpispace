include(pnet_macros)

pnet_compile(pnetv-ufbmig_calc calc.xml
  OUTPUT ufbmig_calc.pnet
  FLAGS --no-inline=1
)
get_target_property(PNET_FILE pnet-pnetv-ufbmig_calc LOCATION)
add_test(NAME test_pnetv_ufbmig_calc COMMAND exit_code_is_nonzero $<TARGET_FILE:pnetv> ${PNET_FILE})
