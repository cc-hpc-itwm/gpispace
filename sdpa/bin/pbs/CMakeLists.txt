add_executable (start-and-fork "start-and-fork.cpp")
target_link_libraries (start-and-fork
  rif
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)
install (TARGETS start-and-fork DESTINATION "bin")
