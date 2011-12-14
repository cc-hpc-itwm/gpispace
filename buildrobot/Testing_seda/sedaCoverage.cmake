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
set(KDE_CTEST_VCS_REPOSITORY https://svn.itwm.fhg.de/svn/SDPA/trunk/seda/trunk)
set(KDE_CTEST_AVOID_SPACES TRUE)
set(KDE_CTEST_BUILD_SUFFIX "gcc")
set(KDE_CTEST_VCS_PATH "")
set(KDE_CTEST_DASHBOARD_DIR "/tmp/seda/coverage")
set(KDE_CTEST_PARALLEL_LEVEL 1)
set(MODULES_VCS https://svn.itwm.fhg.de/svn/SDPA/trunk/modules/trunk)

# for now hardcode the generator to "Unix Makefiles"
#set(CTEST_CMAKE_GENERATOR "Unix Makefiles" )
#set(CTEST_USE_LAUNCHERS 1)
set (CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE)

# generic support code, provides the kde_ctest_setup() macro, which sets up everything required:
get_filename_component(_currentDir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include( "${_currentDir}/KDECTestNightly.cmake")
include( "${_currentDir}/sedaConfig.cmake")

find_program(CTEST_COVERAGE_COMMAND NAMES gcov)
find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)

kde_ctest_setup()

# now actually do the Nightly
ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")

ctest_start(Coverage)
ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}" RETURN_VALUE res)
execute_process(COMMAND svn co ${MODULES_VCS} ${CTEST_SOURCE_DIRECTORY}/modules)
message(STATUS "ctest_update: res='${res}'")

# read additional settings, like maximum number warnings, warning exceptions, etc.
include("${CTEST_SOURCE_DIRECTORY}/CTestConfig.cmake")
include("${CTEST_SOURCE_DIRECTORY}/CTestCustom.cmake" OPTIONAL)


# if CMAKE_INSTALL_PREFIX and BUILD_experimental were defined on the command line, put them
# in the initial cache, so cmake gets them
set(BOOST_ROOT /home/projects/genlm/external_software/boost/1.3.7)
set(FHGLOG_HOME  "${FHGLOG_LATEST}")
set(WITH_FHGLOG  "${SEDA_FHGLOG}")
set(WITH_LOG4CPP "${SEDA_LOG4CPP}")
set(WITH_COMM    "${SEDA_COMM}")
set(ENABLE_CODECOVERAGE 1)

set(CMAKE_BUILD_TYPE Profile)

kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}"
	BOOST_ROOT
        FHGLOG_HOME
        WITH_FHGLOG
        WITH_LOG4CPP
        WITH_COMM
	ENABLE_CODECOVERAGE
	CMAKE_BUILD_TYPE)

# configure, build, test, submit
ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
message(STATUS "===> ctest_configure: res='${res}'")

include("${CTEST_BINARY_DIRECTORY}/CTestCustom.cmake" OPTIONAL)

ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
message(STATUS "===> ctest_build: res='${res}'")

# now run all tests:
ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
message(STATUS "===> ctest_test: res='${res}'")

# do codecoverage now
ctest_coverage(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
message(STATUS "===> ctest_coverage: res='${res}'")

# do codecoverage now
ctest_memcheck(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
message(STATUS "===> ctest_memcheck: res='${res}'")

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
