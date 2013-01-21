# -*- mode: cmake; -*-

set(ENV{https_proxy} "http://squid.itwm.fhg.de:3128/")

set(CTEST_ADDITIONAL_SUFFIX "gcc")
set(ENABLE_GPI_SPACE Yes)

#if(NOT COMPILER_ID)
#  set(KDE_CTEST_BUILD_SUFFIX "clang")
#else(NOT COMPILER_ID)
#  if( ${COMPILER_ID} MATCHES "gcc")
#    set(KDE_CTEST_BUILD_SUFFIX "gcc")
#  else( ${COMPILER_ID} MATCHES "gcc")
#    if( ${COMPILER_ID} MATCHES "clang")
#      set(KDE_CTEST_BUILD_SUFFIX "clang")
#    else( ${COMPILER_ID} MATCHES "clang")
#      if( ${COMPILER_ID} MATCHES "intel")
#        set(KDE_CTEST_BUILD_SUFFIX "intel")
#        set(CTEST_ADDITIONAL_SUFFIX "intel")
#        set(ENABLE_GPI_SPACE No)
#
#      else( ${COMPILER_ID} MATCHES "intel")
#        set(KDE_CTEST_BUILD_SUFFIX "gcc")
#      endif( ${COMPILER_ID} MATCHES "intel")
#    endif( ${COMPILER_ID} MATCHES "clang")
#  endif( ${COMPILER_ID} MATCHES "gcc")
#endif(NOT COMPILER_ID)

#message("Suffix: '${KDE_CTEST_BUILD_SUFFIX}'")
#set(CTEST_BUILD_ARCH "linux")
#set(CTEST_BUILD_NAME "${CTEST_BUILD_ARCH}-${KDE_CTEST_BUILD_SUFFIX}-default")

set(EXTERNAL_SOFTWARE /home/projects/sdpa/external_software)

set(ENABLE_SDPA_GPI No)
set(GPI_PRIV_DIR /tmp)
set(WE_PRECOMPILE OFF)
#set(CMAKE_BUILD_TYPE Release)

set(WITH_FAKE_PC On)

set(SEDA_FHGLOG On)
set(SEDA_LOG4CPP Off)
set(SEDA_COMM  Off)

set(BOOST_ROOT ${EXTERNAL_SOFTWARE}/boost/1.45/${CTEST_ADDITIONAL_SUFFIX})
set(BOOST_FILESYSTEM_VERSION 3)

set(SMC_HOME ${EXTERNAL_SOFTWARE}/smc/5.0.0)


set(APPLICATION_LATEST ${EXTERNAL_SOFTWARE}/latest/application)
# set(BUILDROBOT_LATEST  ${EXTERNAL_SOFTWARE}/latest/buildrobot )
set(EDITOR_LATEST      ${EXTERNAL_SOFTWARE}/latest/editor)
set(FHGCOM_LATEST      ${EXTERNAL_SOFTWARE}/latest/fhgcom)
set(FHGLOG_LATEST      ${EXTERNAL_SOFTWARE}/latest/fhglog)
set(FUSE_LATEST        ${EXTERNAL_SOFTWARE}/latest/fuse)
set(FVM_LATEST         ${EXTERNAL_SOFTWARE}/latest/fvm-pc)
set(GPI-SPACE_LATEST   ${EXTERNAL_SOFTWARE}/latest/gpi-space)
# set(MAIN_LATEST        ${EXTERNAL_SOFTWARE}/latest/)
set(MMGR_LATEST        ${EXTERNAL_SOFTWARE}/latest/mmgr)
# set(MODULES_LATEST     ${EXTERNAL_SOFTWARE}/latest/)
set(MONITOR_LATEST     ${EXTERNAL_SOFTWARE}/latest/monitor)
set(PLAYGROUND_LATEST  ${EXTERNAL_SOFTWARE}/latest/playground)
set(REWRITE_LATEST     ${EXTERNAL_SOFTWARE}/latest/rewrite)
set(SDPA_LATEST        ${EXTERNAL_SOFTWARE}/latest/sdpa)
set(SDPA-GPI_LATEST    ${EXTERNAL_SOFTWARE}/latest/sdpa-gpi)
set(SDPA-GUI_LATEST    ${EXTERNAL_SOFTWARE}/latest/sdpa-gui)
set(SEDA_LATEST        ${EXTERNAL_SOFTWARE}/latest/seda)
set(UTIL_LATEST        ${EXTERNAL_SOFTWARE}/latest/util)
set(WE_LATEST          ${EXTERNAL_SOFTWARE}/latest/we)
set(XML_LATEST         ${EXTERNAL_SOFTWARE}/latest/xml)

if( EXISTS ${EXTERNAL_SOFTWARE}/graphviz/2.24 )
  set(GRAPHVIZ_HOME      ${EXTERNAL_SOFTWARE}/graphviz/2.24)
endif( EXISTS ${EXTERNAL_SOFTWARE}/graphviz/2.24 )

# set pathnames for QT
set(QTDIR "${EXTERNAL_SOFTWARE}/qt/4.7.1")
set(ENV{QTDIR} ${QTDIR})
set(ENV{PATH} "${QTDIR}/bin:$ENV{PATH}")
set(ENV{PKG_CONFIG_PATH} "${QTDIR}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
set(ENV{BOOST_ROOT} "${BOOST_ROOT}")
