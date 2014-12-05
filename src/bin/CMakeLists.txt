# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (vmem.cpp
  LINK_LIBRARIES fhg-util
                 gspc
                 ${FhgLog_LIBRARY}
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

find_package (LibReadline REQUIRED QUIET)
include_directories (SYSTEM ${LibReadline_INCLUDE_DIR})

fhg_add_runtime_executable (gpish.cpp
  LINK_LIBRARIES gpi-space-pc-client-static
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

set_target_properties (gpish
  PROPERTIES
  INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
)

fhg_add_runtime_executable (gpi-space.cpp
  LINK_LIBRARIES gpi-space-gpiapi
                 gpi-space-pc-container
                 gpi-space-pc-global
                 gpi-space-pc-memory
                 gpi-space-pc-segment
                 fhg-revision
                 fhg-util
                 ${MMGR_LIBRARY}
                 ${FhgCom_LIBRARY}
                 ${FhgLog_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
                 ${Boost_FILESYSTEM_LIBRARY}
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
  we-exec.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 we-dev # for the client only
                 fhg-plugin
                 sdpa
                 fake-drts-kernel-plugin
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

fhg_add_runtime_executable (
  pnetv.cpp
  LINK_LIBRARIES fhg-revision
                 pnetv-jpna
                 pnetv-jpn
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
)
