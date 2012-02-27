# This is a script for running the Nightly build and testing of PreStack Pro.
# It is executed via cron on vr3 as user pspro.
#
# It uses the file KDECTestNightly.cmake, which is in KDE svn in kdesdk/cmake/modules/. 
#
# At the bottom of this file you can find  (commented out) a simple shell script which 
# I use to drive the Nightly builds on my machine. You have to adapt this to the 
# conditions on your system, then you can run it e.g. via cron.
#
# Alex <neundorf AT kde.org>

# The PreStack Pro repository:
set(KDE_CTEST_VCS svn)
set(KDE_CTEST_VCS_REPOSITORY https://svn.itwm.fhg.de/svn/SDPA)
set(KDE_CTEST_AVOID_SPACES TRUE)
set(KDE_CTEST_BUILD_SUFFIX "gcc")
#set(KDE_CTEST_VCS_PATH ".")
set(KDE_CTEST_VCS_PATH "trunk")
set(KDE_CTEST_DASHBOARD_DIR "/tmp/SDPA/nightly")
set(KDE_CTEST_PARALLEL_LEVEL 8)
##set(CTEST_BINARY_DIRECTORY ${KDE_CTEST_DASHBOARD_DIR})

# for now hardcode the generator to "Unix Makefiles"
#set(CTEST_CMAKE_GENERATOR "Unix Makefiles" )
#set(CTEST_USE_LAUNCHERS 1)
execute_process(COMMAND uname -n OUTPUT_VARIABLE CMAKE_HOST_NAME)

# generic support code, provides the kde_ctest_setup() macro, which sets up everything required:
get_filename_component(_currentDir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include( "${_currentDir}/KDECTestNightly.cmake")
include( "${_currentDir}/SDPAConfig.cmake")

kde_ctest_setup()

# now actually do the Nightly
ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")
# ctest_start(Continuous)
ctest_start(Nightly)
ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}" )
set(CTEST_SOURCE_DIRECTORY "${CTEST_SOURCE_DIRECTORY}")

# read additional settings, like maximum number warnings, warning exceptions, etc.
include("${CTEST_SOURCE_DIRECTORY}/CTestConfig.cmake")
include("${CTEST_SOURCE_DIRECTORY}/CTestCustom.cmake" OPTIONAL)


# if CMAKE_INSTALL_PREFIX and BUILD_experimental were defined on the command line, put them
# in the initial cache, so cmake gets them
#set(QT_QMAKE_EXECUTABLE /p/hpc/psp/Qt/Qt-4.6.2-icc64-fileenginewatcher/bin/qmake )
set(CMAKE_BUILD_TYPE Release)
set(WE_PRECOMPILE Off)

kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}" QT_QMAKE_EXECUTABLE
	BOOST_ROOT SMC_HOME ENABLE_SDPA_GPI ENABLE_GPI_SPACE GPI_PRIV_DIR
	WE_PRECOMPILE
	GRAPHVIZ_HOME
	CMAKE_BUILD_TYPE)

message("Compiler: '${CXX_COMPILER_ID}'")
message("Compiler: '${CTEST_CXX_COMPILER}'")

# configure, build, test, submit
ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}"  RETURN_VALUE resultConfigure)
message("====> Configure: ${resultConfigure}")

if ( NOT ${resultConfigure} )
  include("${CTEST_BINARY_DIRECTORY}/CTestCustom.cmake" OPTIONAL)

  ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
  message("====> BUILD: ${res}")

  # now run all tests:
  ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE resultTest)
  message("====> ctest_test: res='${resultTest}'")

  if( NOT ${resultTest} )
    include (${CTEST_BINARY_DIRECTORY}/CPackConfig.cmake)

    # can package software
    execute_process(COMMAND make package
          WORKING_DIRECTORY "${CTEST_BINARY_DIRECTORY}"
          RESULT_VARIABLE resultPackage
          )
    message("====> ctest_package: res='${resultPackage}'")
 
    set(UPLOAD_DIR "packages/${CMAKE_HOST_NAME}")
    set(UPLOAD_HOST p800hpc03)
 
    # upload package
    execute_process(COMMAND ssh ${UPLOAD_HOST} mkdir -p ${UPLOAD_DIR}
          )
    execute_process(COMMAND scp -p ${CPACK_PACKAGE_FILE_NAME}.tar.bz2 ${UPLOAD_HOST}:${UPLOAD_DIR}
          WORKING_DIRECTORY "${CTEST_BINARY_DIRECTORY}"
          RESULT_VARIABLE resultUpload
          )
    message("====> ctest_package: res='${resultUpload}'")
 
  endif( NOT ${resultTest} )
else ( NOT ${resultConfigure} )
  message("Configure failed. skip other tasks.")
endif ( NOT ${resultConfigure} )

# submit the build and test results to cdash:
#ctest_submit()
