if (BUILD_TESTING)
  include(SDPAFindBoost)

  set (prefixes
    die
    dir_build
    env
    error
    measured
    measurement
  )

  foreach (prefix ${prefixes})
    configure_file (${prefix}.sh.in ${prefix}.sh @ONLY)
  endforeach()
endif()
