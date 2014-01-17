# bernd.loerwald@itwm.fraunhofer.de

include_directories (${CMAKE_SOURCE_DIR}/sdpa/plugins) #wfe

include(fhg_plugin_macros)
fhg_add_plugin(drts drts.cpp job.cpp drts_callbacks.cpp
  LINK_LIBRARIES sdpa
                 ${FhgCom_LIBRARY}
                 ${GSPC_LIBRARY_SHARED}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
  HEADERS drts_callbacks.h
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
