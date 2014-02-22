# bernd.loerwald@itwm.fraunhofer.de

add_executable(fhgcomd fhgcomd.cpp)
target_link_libraries(fhgcomd fhgcom ${FhgLog_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_THREAD_LIBRARY})

add_executable(fhgcomc fhgcomc.cpp)
target_link_libraries(fhgcomc fhgcom ${FhgLog_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_THREAD_LIBRARY})

add_executable(fhgkvsd fhgkvsd.cpp)
target_link_libraries(fhgkvsd fhgcom ${FhgLog_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SERIALIZATION_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_THREAD_LIBRARY})

add_executable(fhgkvsc fhgkvsc.cpp)
target_link_libraries(fhgkvsc fhgcom ${FhgLog_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SERIALIZATION_LIBRARY} ${Boost_THREAD_LIBRARY})

add_executable(fhgaddr fhgaddr.cpp)
target_link_libraries(fhgaddr fhgcom ${FhgLog_LIBRARY})

install(TARGETS fhgkvsd
  RUNTIME
  DESTINATION bin
)

install(TARGETS fhgkvsc
  RUNTIME
  DESTINATION bin
)

install(TARGETS fhgaddr
  RUNTIME
  DESTINATION bin
)

find_package (LibReadline REQUIRED QUIET)
include_directories (SYSTEM ${LibReadline_INCLUDE_DIR})

add_executable (gpish gpish.cpp)
target_link_libraries (gpish
  gpi-space-pc-client-static
  gpi-space-pc-segment
  ${LibReadline_LIBRARY}
  ${FhgLog_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
  fhg-revision
)

set_target_properties(gpish
  PROPERTIES
  INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
  )
install (TARGETS gpish
  RUNTIME
  DESTINATION bin
)

include (fhg_executable_macros)

fhg_add_runtime_executable (
  we-exec.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 we-dev # for the client only
                 ${FhgLog_LIBRARY}
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_DL_LIBS}
)

fhg_add_runtime_executable (
  pnetget.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnetput.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnet2dot.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnetc.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)
