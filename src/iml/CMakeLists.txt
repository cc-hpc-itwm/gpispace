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

# \todo which minimum version?
# centos5: 2.7.4
# centos6: 2.8.3, epel: 3.6.1
# centos7: 2.9.2, epel: 3.6.1
# centos8: 2.9.7, 3.2.1
# debian9: 2.9.7
# debian10: 2.9.9, 3.4.1
# debian11: 2.9.9, 3.9.3
# ubuntu16: 2.9.4
# ubuntu18: 2.9.7
# ubuntu20: 2.9.9, 3.9.0
# oldest version still in current release notes: 2.9.4 (2015)
# ITWM lts installations: 2.9.2 (non-dev 3.6.1 only!)
# operations not available (compared to 2.9.2):
#   2.8
#   - poll
#   - ioctl
#   2.9
#   - read_buf
#   - write_buf
#   - flock
#   2.9.1
#   - fallocate
# of which poll, read_buf and write_buf feel relevant for "better
# performance" and ioctl for non-posix features.
# \todo make optional (pkg-config wrapper does not support that yet)
# and only build when available
find_package (libfuse 2.7.4 REQUIRED)

extended_add_executable (NAME iml-fuse-client
  SOURCES "fuse/iml-fuse-client.cpp"
  LIBRARIES IML::vmem-pc-client-static
            Util::Generic
            libfuse::libfuse
            Boost::filesystem
)
