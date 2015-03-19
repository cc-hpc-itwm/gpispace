# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (gpi-space.cpp
  LINK_LIBRARIES gpi-space-gpiapi
                 gpi-space-pc-container
                 gpi-space-pc-global
                 gpi-space-pc-memory
                 gpi-space-pc-segment
                 fhg-revision
                 fhg-util
                 mmgr
                 fhgcom
                 fhglog
                 rif-startup-messages-pipe
                 Util::Generic
                 ${CMAKE_THREAD_LIBS_INIT}
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

set_target_properties (gpi-space
  PROPERTIES
  INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
)

install (TARGETS gpi-space
  RUNTIME
  DESTINATION bin
)

fhg_add_runtime_executable (
  pnet2dot.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 Util::Generic
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnetc.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 Util::Generic
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnetv.cpp
  LINK_LIBRARIES fhg-revision
                 pnetv-jpna
                 pnetv-jpn
                 pnet
                 Util::Generic
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
)
