  find_package (HWLOC REQUIRED QUIET)

  set (prefixes
    die
    dir_build
    dir_state
    env
    error
    measured
    measurement
  )

  foreach (prefix ${prefixes})
    configure_file (${prefix}.sh.in ${prefix}.sh @ONLY)
  endforeach()
