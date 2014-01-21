# bernd.loerwald@itwm.fraunhofer.de

find_package (HWLOC REQUIRED QUIET)
include_directories (SYSTEM ${HWLOC_INCLUDE_DIR})

include(fhg_plugin_macros)

fhg_add_plugin(drts drts.cpp job.cpp
  LINK_LIBRARIES sdpa
                 ${FhgCom_LIBRARY}
                 ${GSPC_LIBRARY_SHARED}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
  HEADERS drts_callbacks.h logc.hpp magically_available_functions.hpp
)

add_executable (drts-kernel drts-kernel.cpp)
target_link_libraries (drts-kernel
  fhg-plugin
  we-dev
  ${UTIL_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_THREAD_LIBRARY})

install (TARGETS drts-kernel
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
