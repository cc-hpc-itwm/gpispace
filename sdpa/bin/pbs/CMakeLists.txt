set (scripts
  start-sdpa
  sdpa-selftest
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

add_executable (sdpa-binary "sdpa.cpp")
target_link_libraries (sdpa-binary
  sdpa
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
)
set_target_properties (sdpa-binary PROPERTIES OUTPUT_NAME "sdpa")
install (TARGETS sdpa-binary DESTINATION "bin")

add_executable (start-drts "start-drts.cpp")
target_link_libraries (start-drts
  start_drts_kernel
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
)
install (TARGETS start-drts DESTINATION "libexec/sdpa/scripts")
