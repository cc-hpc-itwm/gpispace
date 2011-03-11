# -*- mode: cmake; -*-
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
set(KDE_CTEST_VCS_PATH "trunk")
set(KDE_CTEST_DASHBOARD_DIR "/tmp/SDPA/coverage")
set(KDE_CTEST_PARALLEL_LEVEL 8)
##set(CTEST_BINARY_DIRECTORY ${KDE_CTEST_DASHBOARD_DIR}/main/trunk)

# for now hardcode the generator to "Unix Makefiles"
#set(CTEST_CMAKE_GENERATOR "Unix Makefiles" )
#set(CTEST_USE_LAUNCHERS 1)
#set (CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE)
set(CTEST_TEST_TIMEOUT 1800)

# generic support code, provides the kde_ctest_setup() macro, which sets up everything required:
get_filename_component(_currentDir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include( "${_currentDir}/KDECTestNightly.cmake")

find_program(CTEST_COVERAGE_COMMAND NAMES gcov)
find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)

kde_ctest_setup()

# now actually do the Nightly
ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")
ctest_start(Coverage)
ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}" RETURN_VALUE res)
set(CTEST_SOURCE_DIRECTORY "${CTEST_SOURCE_DIRECTORY}/main/trunk")
message(STATUS "ctest_update: res='${res}'")

# read additional settings, like maximum number warnings, warning exceptions, etc.
include("${CTEST_SOURCE_DIRECTORY}/CTestConfig.cmake")
include("${CTEST_SOURCE_DIRECTORY}/CTestCustom.cmake" OPTIONAL)


# if CMAKE_INSTALL_PREFIX and BUILD_experimental were defined on the command line, put them
# in the initial cache, so cmake gets them
set(BOOST_ROOT /opt/boost/1.45/gcc)
set(SMC_HOME /opt/smc/5.0.0/)
set(ENABLE_SDPA_GPI No)
set(ENABLE_GPI_SPACE Yes)
set(GPI_PRIV_DIR /tmp/)
set(CMAKE_BUILD_TYPE Profile)
set(ENABLE_CODECOVERAGE 1)

kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}" QT_QMAKE_EXECUTABLE
        BOOST_ROOT SMC_HOME ENABLE_SDPA_GPI ENABLE_GPI_SPACE GPI_PRIV_DIR
	ENABLE_CODECOVERAGE
        CMAKE_BUILD_TYPE)

# configure, build, test, submit
ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
message(STATUS "===> ctest_configure: res='${res}'")

include("${CTEST_BINARY_DIRECTORY}/CTestCustom.cmake" OPTIONAL)

ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
message(STATUS "===> ctest_build: res='${res}'")

#execute_process(COMMAND make coverage_init
#	WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY})

# do codecoverage now
ctest_coverage(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
message(STATUS "===> ctest_coverage: res='${res}'")

# do codecoverage now
ctest_memcheck(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
message(STATUS "===> ctest_memcheck: res='${res}'")

# now run all tests:
ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
message(STATUS "===> ctest_test: res='${res}'")

#execute_process(COMMAND make coverage
#	WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY})

# submit the build and test results to cdash:
ctest_submit()


#return()

## now build the doxygen docs
#execute_process(COMMAND make -C "${CTEST_BINARY_DIRECTORY}" alldocs -j2)
#
## ...and copy them to the pubdoc directory:
#execute_process(COMMAND cp -R "${CTEST_SOURCE_DIRECTORY}/BackEnd/html-apidocs/" /p/hpc/public_html/pspro/dox/BackEnd/ )
#execute_process(COMMAND cp -R "${CTEST_SOURCE_DIRECTORY}/BackEnd/html-details/" /p/hpc/public_html/pspro/dox/BackEnd/ )
#
#execute_process(COMMAND cp -R "${CTEST_SOURCE_DIRECTORY}/RemoteView/html-apidocs/" /p/hpc/public_html/pspro/dox/RemoteView/ )
#execute_process(COMMAND cp -R "${CTEST_SOURCE_DIRECTORY}/RemoteView/html-details/" /p/hpc/public_html/pspro/dox/RemoteView/ )
