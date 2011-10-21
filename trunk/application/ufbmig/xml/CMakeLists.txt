include(pnet_macros)

pnet_compile(ufbmig_init init.xml
  OUTPUT ufbmig_init.pnet GENERATE ufbmig_init
)
pnet_compile(ufbmig_calc calc.xml
  INCLUDES ${CMAKE_SOURCE_DIR}/application
  OUTPUT ufbmig_calc.pnet GENERATE ufbmig_init
)
pnet_compile(ufbmig_done finalize.xml
  OUTPUT ufbmig_done.pnet GENERATE ufbmig_init
)
