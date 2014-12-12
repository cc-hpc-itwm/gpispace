set (prgs
  sdpa
)

set (scripts
  start-sdpa
  sdpa-selftest
)

install(PROGRAMS ${prgs}
  DESTINATION bin
)

install(PROGRAMS ${scripts}
  DESTINATION libexec/sdpa/scripts
)

add_executable (start-and-fork "start-and-fork.cpp")
target_link_libraries (start-and-fork
  rif
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)
install (TARGETS start-and-fork DESTINATION "bin")

add_executable (start-drts "start-drts.cpp")
target_link_libraries (start-drts
  start_drts_kernel
  ${Boost_PROGRAM_OPTIONS_LIBRARY}

)
install (TARGETS start-drts DESTINATION "libexec/sdpa/scripts")
