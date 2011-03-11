# This is a script for running the Nightly build and testing of PreStack Pro.
# It is executed via cron on vr3 as user pspro.
#
# It uses the file KDECTestNightly.cmake, which is in KDE svn in kdesdk/cmake/modules/. 
# You need to have this file on some location on your system and then point the environment variable
# KDECTESTNIGHTLY_DIR to the directory containing this file when running this script.
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
#set(KDE_CTEST_VCS_PATH "main/trunk")
set(KDE_CTEST_VCS_PATH "trunk")
set(KDE_CTEST_DASHBOARD_DIR "/tmp/SDPA/continuous")
set(KDE_CTEST_PARALLEL_LEVEL 8)
##set(CTEST_BINARY_DIRECTORY ${KDE_CTEST_DASHBOARD_DIR}/main/trunk)


# for now hardcode the generator to "Unix Makefiles"
#set(CTEST_CMAKE_GENERATOR "Unix Makefiles" )
#set(CTEST_USE_LAUNCHERS 1)

# generic support code, provides the kde_ctest_setup() macro, which sets up everything required:
get_filename_component(_currentDir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include( "${_currentDir}/KDECTestNightly.cmake")

kde_ctest_setup()


ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")
# if CMAKE_INSTALL_PREFIX and BUILD_experimental were defined on the command line, put them
# in the initial cache, so cmake gets them
#set(QT_QMAKE_EXECUTABLE /p/hpc/psp/QT-4.5.2-icc64/bin/qmake )
#set(SQUISH_INSTALL_DIR /p/hpc/psp/squish-20100224-qt45x-linux64 )
#set(PSP_ENABLE_SQUISH_TESTS TRUE)
set(BOOST_ROOT /opt/boost/1.45/gcc)
set(SMC_HOME /opt/smc/5.0.0/)
set(ENABLE_SDPA_GPI No)
set(ENABLE_GPI_SPACE Yes)
set(GPI_PRIV_DIR /tmp)
set(WE_PRECOMPILE OFF)
set(CMAKE_BUILD_TYPE Release)
kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}" QT_QMAKE_EXECUTABLE
        BOOST_ROOT SMC_HOME ENABLE_SDPA_GPI ENABLE_GPI_SPACE GPI_PRIV_DIR
        CMAKE_BUILD_TYPE WE_PRECOMPILE)

set(SDPA_TOPSRCDIR "${CTEST_SOURCE_DIRECTORY}")

set(firstLoop TRUE)

while (${CTEST_ELAPSED_TIME} LESS 43200)
  message("Running next loop...")
  set (START_TIME ${CTEST_ELAPSED_TIME})

  ctest_start(Continuous)
  ctest_update(SOURCE "${SDPA_TOPSRCDIR}" RETURN_VALUE updatedFiles)
  set(CTEST_SOURCE_DIRECTORY "${SDPA_TOPSRCDIR}/main/trunk")
   
  if ("${updatedFiles}" GREATER 0  OR  firstLoop)

    # read additional settings, like maximum number warnings, warning exceptions, etc.
    include("${CTEST_SOURCE_DIRECTORY}/CTestConfig.cmake")
    include("${CTEST_SOURCE_DIRECTORY}/CTestCustom.cmake" OPTIONAL)

    # configure, build, test, submit
    ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}" )

    include("${CTEST_BINARY_DIRECTORY}/CTestCustom.cmake" OPTIONAL)

    ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" )


    # now run all tests:
    ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" )

    # submit the build and test results to cdash:
    ctest_submit()
    set(firstLoop FALSE)
  endif ("${updatedFiles}" GREATER 0  OR  firstLoop)

  ctest_sleep( 120) # ${START_TIME} 300 ${CTEST_ELAPSED_TIME})

endwhile()


