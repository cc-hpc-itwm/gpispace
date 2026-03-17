###############################################################################
# CMake utility modules for GPI-Space
###############################################################################

list (APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

set (CMAKE_MODULE_PATH
  "${CMAKE_MODULE_PATH}"
  PARENT_SCOPE
)

###############################################################################
# Install cmake modules
###############################################################################

set (_cmake_install_dir "${GSPC_SHARE_INSTALL_DIR}/cmake")

install (
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/_bundle.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/add_cxx_compiler_flag_if_supported.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/add_macros.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/install_directory.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/parse_arguments.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/_test_wrapper.cmake"
  DESTINATION
    "${_cmake_install_dir}"
)
install (
  PROGRAMS
    "${CMAKE_CURRENT_SOURCE_DIR}/bundle.sh"
  DESTINATION
    "${_cmake_install_dir}"
)

