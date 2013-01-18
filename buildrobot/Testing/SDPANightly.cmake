# -*- mode: cmake; -*-

include(Tools.cmake)
my_ctest_setup()
include(CTestConfigSDPA.cmake)
include(SDPAConfig.cmake)
set(_ctest_type "Nightly")
# set(_ctest_type "Continuous")
# set(_ctest_type "Coverage")

set(URL "krueger@login.itwm.fhg.de:/p/hpc/sdpa/repo/sdpa.git")

set(CTEST_BASE_DIRECTORY "${KDE_CTEST_DASHBOARD_DIR}/${_projectNameDir}/${_ctest_type}")
set(CTEST_SOURCE_DIRECTORY "${CTEST_BASE_DIRECTORY}/${_srcDir}-${_git_branch}" )
set(CTEST_BINARY_DIRECTORY "${CTEST_BASE_DIRECTORY}/${_buildDir}-${CTEST_BUILD_NAME}")
set(CTEST_INSTALL_DIRECTORY "${CTEST_BASE_DIRECTORY}/install-${CTEST_BUILD_NAME}")
set(KDE_CTEST_VCS "git")
set(KDE_CTEST_VCS_REPOSITORY ${URL})
set(KDE_CTEST_PARALLEL_LEVEL 8)

#if(NOT CTEST_BUILD_NAME)
#  set(CTEST_BUILD_NAME ${CMAKE_SYSTEM_NAME}-CMake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}${KDE_C)
#endif(NOT CTEST_BUILD_NAME)

set(CMAKE_INSTALL_PREFIX "/usr")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
#set(CTEST_BUILD_CONFIGURATION "Profiling")

configure_ctest_config(${KDE_CTEST_VCS_REPOSITORY} "CTestConfigSDPA.cmake")
kde_ctest_setup()

FindOS(OS_NAME OS_VERSION)

set(ctest_config ${CTEST_BASE_DIRECTORY}/CTestConfig.cmake)
#######################################################################


#execute_process(COMMAND uname -n OUTPUT_VARIABLE CMAKE_HOST_NAME)

#set(KDE_CTEST_DASHBOARD_DIR "/tmp/SDPA/nightly")

## generic support code, provides the kde_ctest_setup() macro, which sets up everything required:
#include(CTestConfigSDPA.cmake)

#set(ctest_config ${CTEST_BASE_DIRECTORY}/CTestConfig.cmake)
#######################################################################
foreach(subproject ${CTEST_PROJECT_SUBPROJECTS})
  ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY}/${subproject})
endforeach()
ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")

find_program(CTEST_GIT_COMMAND NAMES git)
set(CTEST_UPDATE_TYPE git)

set(CTEST_UPDATE_COMMAND  ${CTEST_GIT_COMMAND})
if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}/.git/HEAD")
  set(CTEST_CHECKOUT_COMMAND "${CTEST_GIT_COMMAND} clone ${URL} ${CTEST_SOURCE_DIRECTORY}")
endif(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}/.git/HEAD")

create_project_xml()

configure_file("CTestConfigSDPA.cmake" ${CTEST_BASE_DIRECTORY}/CTestConfig.cmake COPYONLY)
ctest_start(${_ctest_type})
ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}")
ctest_submit(PARTS Update)

execute_process(
  COMMAND ${CTEST_GIT_COMMAND} checkout  ${_git_branch}
  WORKING_DIRECTORY ${CTEST_SOURCE_DIRECTORY}
  )

# to get CTEST_PROJECT_SUBPROJECTS definition:
include(${ctest_config})

##
set(CMAKE_ADDITIONAL_PATH ${CTEST_INSTALL_DIRECTORY})
set(CMAKE_BUILD_TYPE Release)
set(WE_PRECOMPILE Off)

kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}" QT_QMAKE_EXECUTABLE
        BOOST_ROOT SMC_HOME ENABLE_SDPA_GPI ENABLE_GPI_SPACE
        GPI_PRIV_DIR
        WE_PRECOMPILE
        GRAPHVIZ_HOME
        CMAKE_BUILD_TYPE)

#  ##
#  if(CMAKE_TOOLCHAIN_FILE)
#    kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}/${subproject}"
#      CMAKE_TOOLCHAIN_FILE
#      CMAKE_INSTALL_PREFIX
#      CMAKE_ADDITIONAL_PATH
#      )
#  else(CMAKE_TOOLCHAIN_FILE)
##    if( ${subproject} STREQUAL "hexaswitch")
#      set(BOOST_ROOT /homes/krueger/external_software/ubuntu_100403/boost/1.46)
#      kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}/${subproject}"
#	BOOST_ROOT
#	CMAKE_INSTALL_PREFIX
#	CMAKE_ADDITIONAL_PATH
#	)
#  endif(CMAKE_TOOLCHAIN_FILE)
  
  ##
  ctest_configure(BUILD ${CTEST_BINARY_DIRECTORY}
    SOURCE ${CTEST_SOURCE_DIRECTORY}  APPEND
    RETURN_VALUE resultConfigure)
  ctest_submit(PARTS Configure)
  message("====> Configure: ${resultConfigure}")

message("====> ${CTEST_BUILD_FLAGS}")
set(ENV{MAKELEVEL} "1")
set(ENV{MAKEFLAGS} "-j${KDE_CTEST_PARALLEL_LEVEL}")

  set(CTEST_BUILD_TARGET ${subproject})
  ctest_build(BUILD ${CTEST_BINARY_DIRECTORY} 
    APPEND RETURN_VALUE build_res)
  # builds target ${CTEST_BUILD_TARGET}
  ctest_submit(PARTS Build)
  message("====> BUILD: ${build_res}")

  ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}"
    PARALLEL_LEVEL ${KDE_CTEST_PARALLEL_LEVEL}
#    INCLUDE_LABEL "${subproject}"
  )

  # runs only tests that have a LABELS property
  #matching "${subproject}"
  ctest_submit(PARTS Test)

#  ## do an installation
#  if( NOT ${build_res})
#    execute_process(
#      COMMAND cmake -DCMAKE_INSTALL_PREFIX=${CTEST_INSTALL_DIRECTORY} -P cmake_install.cmake
#      WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}/${subproject}
#      )
#  endif( NOT ${build_res})
  
#  ## create packages
#  include(${CTEST_BINARY_DIRECTORY}/${subproject}/CPackConfig.cmake)
#  if( STAGING_DIR)
#    set(ENV{PATH}            ${OPENWRT_STAGING_DIR}/host/bin:$ENV{PATH})
#  endif( STAGING_DIR)

#  if( NOT ${build_res})
#    execute_process(
#      COMMAND cpack -G DEB
#      COMMAND cpack -G TGZ
#      WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}/${subproject}
#      )
#  endif( NOT ${build_res})
  
 # # upload files
 # if( NOT ${build_res})
 #   if(CPACK_ARCHITECTUR)
 #     set(OPKG_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_ARCHITECTUR}")
 #     set(_package_file "${OPKG_FILE_NAME}.ipk")
 #   else(CPACK_ARCHITECTUR)
 #     set(_package_file "${CPACK_PACKAGE_FILE_NAME}.deb")
 #   endif(CPACK_ARCHITECTUR)
 #   message("==> Upload packages - ${_package_file}")
 #   set(_export_host "192.168.9.63")
 #   execute_process(
 #     COMMAND ssh ${_export_host} mkdir -p packages
 #     COMMAND scp -p ${_package_file} ${_export_host}:packages/
 #     WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}/${subproject}
 #     )
 # endif( NOT ${build_res})
#endforeach()

ctest_submit(RETURN_VALUE res)
