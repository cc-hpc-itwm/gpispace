set (IML_PROJECT_CONTACT "gpispace-support@itwm.fraunhofer.de" CACHE STRING "Contact information")

determine_git_revision ("${CMAKE_SOURCE_DIR}" IML_PROJECT_REVISION)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/revision.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp.new @ONLY)

execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp.new
  RESULT_VARIABLE revision_cpp_needs_update
  OUTPUT_QUIET
  ERROR_QUIET
)
if (revision_cpp_needs_update)
  file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp.new ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp)
  file (WRITE "${CMAKE_BINARY_DIR}/revision" "${IML_PROJECT_REVISION}")
endif()

extended_add_library (NAME fhg-revision
  NAMESPACE IML
  SOURCES "${CMAKE_CURRENT_BINARY_DIR}/revision.cpp"
)

extended_add_library (NAME Shared
  NAMESPACE IML
  SOURCES "segment_description.cpp"
  LIBRARIES Util::Generic
)

install (FILES "segment_description.hpp"
               "segment_description.ipp"
  DESTINATION "include/iml"
)

add_subdirectory (util)
add_subdirectory (vmem)
add_subdirectory (rif)
add_subdirectory (client)
add_subdirectory (server)
add_subdirectory (testing)
add_subdirectory (tests)
