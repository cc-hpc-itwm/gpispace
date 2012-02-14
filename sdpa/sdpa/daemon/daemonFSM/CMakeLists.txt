if (USE_SMC_SC)
  add_subdirectory(SMC)
endif (USE_SMC_SC)

if (USE_BOOST_SC)
  add_subdirectory(BSC)
endif (USE_BOOST_SC)

if (USE_BOOST_MSM)
 add_subdirectory(BMSM)
endif (USE_BOOST_MSM)