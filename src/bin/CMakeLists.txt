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
                 Boost::filesystem
                 Boost::program_options
                 Boost::serialization
                 Boost::thread
                 Boost::system
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
                 Boost::program_options
                 Boost::serialization
                 Boost::system
)

fhg_add_runtime_executable (
  pnetc.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 Util::Generic
                 Boost::filesystem
                 Boost::program_options
                 Boost::serialization
                 Boost::system
)

fhg_add_runtime_executable (
  pnetv.cpp
  LINK_LIBRARIES fhg-revision
                 pnetv-jpna
                 pnetv-jpn
                 pnet
                 Util::Generic
                 Boost::program_options
)

fhg_add_runtime_executable (
  pnetd.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 gspc
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)
