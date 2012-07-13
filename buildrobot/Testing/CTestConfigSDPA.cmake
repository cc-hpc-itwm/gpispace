# -*- mode: cmake; -*-
## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   ENABLE_TESTING()
##   INCLUDE(CTest)

set(CTEST_PROJECT_NAME "SDPA")
set(CTEST_NIGHTLY_START_TIME "00:00:00 GMT")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "pubdoc.itwm.fhg.de")
set(CTEST_DROP_LOCATION "/p/hpc/pspro/cdash/submit.php?project=${CTEST_PROJECT_NAME}")
set(CTEST_DROP_SITE_CDASH TRUE)

set(CTEST_PROJECT_SUBPROJECTS
main
fhglog
seda
)

site_name(CTEST_SITE)

set(_git_branch "develop")
set(_git_branch "master")
set(GIT_UPDATE_OPTIONS "pull")

set(_projectNameDir "${CTEST_PROJECT_NAME}")
set(_srcDir "srcdir")
set(_buildDir "builddir")
