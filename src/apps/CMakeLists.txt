include (fhg_executable_macros)

include_directories (${CMAKE_SOURCE_DIR}/sdpa)

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
  pneti.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnetsc.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnetlt.cpp
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
