cmake_minimum_required (VERSION 3.16)
include (${CMAKE_CURRENT_SOURCE_DIR}/project-info.cmake)
project (gspc VERSION ${GSPC_VERSION} LANGUAGES CXX)

set (FMT_DIR "${CMAKE_BINARY_DIR}/3rdparty/fmt")
add_subdirectory (3rdparty)

# Do not deploy with relocatable bundle by default: Users/application
# developers need to be aware of how they deploy, so the default
# should be as plain as possible.
# \note Done before including cmake/add_macros.cmake to overwrite default
# given there.
# Compatibility shim for deprecated CMake cache variables.
function (_gspc_dep_var
            old_name
            new_name
            cache_type
         )
  if (DEFINED ${old_name})
    message (DEPRECATION
        "CMake cache variable '${old_name}' is deprecated and will be "
        "removed by the end of 2026. Use '${new_name}' instead."
      )
    if (NOT DEFINED ${new_name})
      set (${new_name} "${${old_name}}" CACHE ${cache_type} "" FORCE)
      set (${new_name} "${${old_name}}" PARENT_SCOPE)
    endif()
  endif()
endfunction()

_gspc_dep_var (INSTALL_DO_NOT_BUNDLE
  GSPC_INSTALL_DO_NOT_BUNDLE
  BOOL
  )
_gspc_dep_var (INSTALL_RPATH_DIRS
  GSPC_INSTALL_RPATH_DIRS
  STRING
  )
_gspc_dep_var (SHARED_DIRECTORY_FOR_TESTS
  GSPC_SHARED_DIRECTORY_FOR_TESTS
  PATH
  )
_gspc_dep_var (TESTING_RIF_STRATEGY
  GSPC_TESTING_RIF_STRATEGY
  STRING
  )
_gspc_dep_var (TESTING_RIF_STRATEGY_PARAMETERS
  GSPC_TESTING_RIF_STRATEGY_PARAMETERS
  STRING
  )
_gspc_dep_var (IML_TESTING_BEEGFS_DIRECTORY
  GSPC_IML_TESTING_BEEGFS_DIRECTORY
  PATH
  )

if (NOT GSPC_INSTALL_DO_NOT_BUNDLE)
  set (GSPC_INSTALL_DO_NOT_BUNDLE ON CACHE BOOL "")
endif()

# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
list (APPEND CMAKE_MODULE_PATH "$ENV{CMAKE_MODULE_PATH}")

option (GSPC_WITH_MONITOR_APP "Enable the 'gspc-monitor' execution and logging \
monitor application (also known as 'gantt')" ON)

option (GSPC_WITH_IML
  "Enable IML support."
  ON
)

option (GSPC_WITH_EXPRESSION_SHELL
  "Enable the gspc-expression-shell utility."
  OFF
)

include (GNUInstallDirs)

set (GSPC_SHARE_INSTALL_DIR "${CMAKE_INSTALL_DATAROOTDIR}/GPISpace")
set (GSPC_LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
set (GSPC_CONFIG_INSTALL_DIR "${GSPC_SHARE_INSTALL_DIR}/cmake")

file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/gspc")
configure_file (
    "${PROJECT_SOURCE_DIR}/include/gspc/installation_path.hpp.in"
    "${PROJECT_BINARY_DIR}/include/gspc/installation_path.hpp"
    @ONLY
  )

set (GSPC_TEST OFF CACHE BOOL "Build GPI-Space tests")

add_subdirectory (cmake)

include (add_macros)
include (add_unit_test)
include (beautify_find_boost)

set (Boost_NO_SYSTEM_PATHS ON)

if (NOT GSPC_ASSERT_MODE)
  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set (GSPC_ASSERT_MODE 0 CACHE STRING "GSPC_ASSERT_MODE")
  else()
    set (GSPC_ASSERT_MODE 1 CACHE STRING "GSPC_ASSERT_MODE")
  endif()
endif()

add_definitions ("-DGSPC_ASSERT_MODE=${GSPC_ASSERT_MODE}")

set (PNETC_LIB_DIR "${GSPC_SHARE_INSTALL_DIR}/xml/lib")

add_definitions ("-DBOOST_NO_CXX11_SCOPED_ENUMS") # required if linking against boost not built with c++11

# Boost.Thread now throws when preconditions are not met.
# This is the case when calling join() in some tests, while
# joinable() == false.  This define is only a hacky workaround and
# the precondition shall be checked everywhere, where needed.
add_definitions (-DBOOST_THREAD_THROW_IF_PRECONDITION_NOT_SATISFIED)

# gpi-space needs boost::mpl::vector with more than $default elements for state machine
add_definitions ("-DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS")
add_definitions ("-DBOOST_MPL_LIMIT_VECTOR_SIZE=40")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.5")
    message (FATAL_ERROR
      "${PROJECT_NAME} requires compiler GNU"
      " to be of version >= 8.5"
      " (got ${CMAKE_CXX_COMPILER_VERSION})."
    )
  endif()
endif()

set (_boost_minimum_version 1.61.0)
set (_boost_maximum_version 1.63.0)
find_boost (${_boost_minimum_version} REQUIRED COMPONENTS
  chrono
  coroutine
  date_time
  iostreams
  program_options
  random
  serialization
  system
  test_exec_monitor
  thread
  timer
  unit_test_framework
)
if (Boost_VERSION VERSION_GREATER ${_boost_maximum_version})
  message (FATAL_ERROR "${PROJECT_NAME} requires Boost to be in version range >= ${_boost_minimum_version} and <= ${_boost_maximum_version} (got ${Boost_VERSION}).")
endif()

find_package (HWLOC REQUIRED)
if (GSPC_WITH_EXPRESSION_SHELL)
  find_package (LibReadline REQUIRED)
endif()
find_package (LibRT REQUIRED)
if (GSPC_WITH_MONITOR_APP)
  find_package (Qt5 COMPONENTS Core Gui Widgets Xml Network REQUIRED)
endif()
find_package (Threads REQUIRED)
find_package (OpenSSL REQUIRED)

include (CompilerFlags)
include (InstallRequiredSystemLibraries)

include (gspc_add_executable)
include (gspc_add_library)

add_library (GPISpace-header-only INTERFACE)
add_library (GPISpace::header-only ALIAS GPISpace-header-only)
target_include_directories (GPISpace-header-only
  INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

include_directories ("${PROJECT_SOURCE_DIR}/share")
include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}")
include_directories ("${PROJECT_BINARY_DIR}/include")

if (CMAKE_USE_PTHREADS_INIT)
  link_libraries (${CMAKE_THREAD_LIBS_INIT} rt)
endif()

include (CheckCXXSourceRuns)

check_cxx_source_runs ([[
  #include <memory>
  int main (int, char**)
  {
    char c;
    void* ptr (&c);
    std::size_t space (sizeof (c));

    return std::align (1, 0, ptr, space) ? 0 : 1;
    //     ^^^^^^^^^^
  }
  ]]
  HAS_STD_ALIGN
)

bundle_boost (DESTINATION "external/boost")
list (APPEND GSPC_INSTALL_RPATH_DIRS "${GSPC_LIBRARY_INSTALL_DIR}")

add_subdirectory (share)
add_subdirectory (src)

if (GSPC_TEST)
  include (CTest)

  set (required_string_option_sentinel_value "required_string_option_sentinel_value-NOT_SET")

  SET (GSPC_SHARED_DIRECTORY_FOR_TESTS
    ${required_string_option_sentinel_value}
    CACHE PATH
    "shared directory for tests"
  )
  if (GSPC_SHARED_DIRECTORY_FOR_TESTS STREQUAL
      ${required_string_option_sentinel_value})
    message (FATAL_ERROR
      "required variable GSPC_SHARED_DIRECTORY_FOR_TESTS not set"
    )
  endif()

  # The strategy tests use to start gspc-rifd.
  SET (GSPC_TESTING_RIF_STRATEGY "ssh"
    CACHE STRING "strategy to use for starting scoped rif daemons"
  )
  # Additional options for the gspc-rifd startup strategy.
  SET (GSPC_TESTING_RIF_STRATEGY_PARAMETERS ""
    CACHE STRING "strategy parameters to use for starting scoped rif daemons"
  )

  if (GSPC_WITH_IML)
    # Might not be a BeeGFS directory, but IML will check that.
    set (GSPC_IML_TESTING_BEEGFS_DIRECTORY
      ${GSPC_SHARED_DIRECTORY_FOR_TESTS}
      CACHE PATH
      "(Shared) directory to use for BeeGFS tests."
    )
  endif()

  add_subdirectory (test)
  add_subdirectory (testing)
  add_subdirectory (system_test)
endif()

install (
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
  DESTINATION
    "${GSPC_SHARE_INSTALL_DIR}"
)

set (GSPC_HOME ".")

set (GPISpace_installconfig_boost_coroutine_warning_suppression)
if (Boost_VERSION VERSION_GREATER 1.61.0 AND Boost_VERSION VERSION_LESS 1.70.0)
  if (Boost_VERSION VERSION_EQUAL 1.62.0)
    set (GPISpace_installconfig_boost_coroutine_warning_suppression [==[
# With Boost 1.62, Boost.Coroutine is deprecated and should be
# replaced with Boost.Coroutine2. Boost.Asio never did, so suppress
# the warning. In 1.70 they removed the warning again.
target_compile_definitions (Boost::boost INTERFACE
  BOOST_COROUTINES_NO_DEPRECATION_WARNING
  BOOST_COROUTINE_NO_DEPRECATION_WARNING
)]==])
  else()
    set (GPISpace_installconfig_boost_coroutine_warning_suppression [==[
# With Boost 1.62, Boost.Coroutine is deprecated and should be
# replaced with Boost.Coroutine2. Boost.Asio never did, so suppress
# the warning. In 1.70 they removed the warning again.
target_compile_definitions (Boost::boost INTERFACE
  BOOST_COROUTINES_NO_DEPRECATION_WARNING
)]==])
  endif()
endif()

include (CMakePackageConfigHelpers)
configure_package_config_file (GPISpaceConfig.cmake.in GPISpaceConfig.cmake
  INSTALL_DESTINATION "${GSPC_CONFIG_INSTALL_DIR}"
  PATH_VARS
    GSPC_HOME
)
write_basic_package_version_file (GPISpaceConfigVersion.cmake
  COMPATIBILITY SameMinorVersion
)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/GPISpaceConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/GPISpaceConfigVersion.cmake"
  DESTINATION "${GSPC_CONFIG_INSTALL_DIR}"
)

install (FILES
  include/gspc/detail/export.hpp
  include/gspc/detail/export.ipp
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/detail"
)
install (FILES
  include/gspc/iml/macros.hpp
  include/gspc/iml/stubs.hpp
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/gspc/iml"
)
