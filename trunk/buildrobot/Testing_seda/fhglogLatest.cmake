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
set(KDE_CTEST_VCS_REPOSITORY https://svn.itwm.fhg.de/svn/SDPA/trunk/fhglog/trunk)
set(KDE_CTEST_AVOID_SPACES TRUE)
set(KDE_CTEST_BUILD_SUFFIX "gcc")
set(KDE_CTEST_VCS_PATH "")
set(KDE_CTEST_DASHBOARD_DIR "/tmp/seda/fhglog")
set(KDE_CTEST_PARALLEL_LEVEL 1)
set(MODULES_VCS https://svn.itwm.fhg.de/svn/SDPA/trunk/modules/trunk)

# for now hardcode the generator to "Unix Makefiles"
#set(CTEST_CMAKE_GENERATOR "Unix Makefiles" )
#set(CTEST_USE_LAUNCHERS 1)

# generic support code, provides the kde_ctest_setup() macro, which sets up everything required:
get_filename_component(_currentDir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include( "${_currentDir}/KDECTestNightly.cmake")
include( "${_currentDir}/sedaConfig.cmake")

kde_ctest_setup()

# now actually do the Nightly
ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")
ctest_start(Latest)
ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}" RETURN_VALUE res )

# checkout cmake modules
execute_process(COMMAND svn co ${MODULES_VCS} ${CTEST_SOURCE_DIRECTORY}/modules)

# read additional settings, like maximum number warnings, warning exceptions, etc.
include("${CTEST_SOURCE_DIRECTORY}/CTestConfig.cmake")
include("${CTEST_SOURCE_DIRECTORY}/CTestCustom.cmake" OPTIONAL)
set(CTEST_DROP_LOCATION "/p/hpc/pspro/cdash/submit.php?project=GenLM&subproject=fhglog")


# if CMAKE_INSTALL_PREFIX and BUILD_experimental were defined on the command line, put them
# in the initial cache, so cmake gets them
set(CMAKE_INSTALL_PREFIX "${FHGLOG_LATEST}")
set(CMAKE_BUILD_TYPE Release)

kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}"
	CMAKE_INSTALL_PREFIX
        BOOST_ROOT
        CMAKE_BUILD_TYPE)

# configure, build, test, submit
ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res )

include("${CTEST_BINARY_DIRECTORY}/CTestCustom.cmake" OPTIONAL)

ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res )

# now run all tests:
ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res )

# submit the build and test results to cdash:
#ctest_submit()

#return()

# run make install
execute_process(COMMAND make install
	WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}
)
