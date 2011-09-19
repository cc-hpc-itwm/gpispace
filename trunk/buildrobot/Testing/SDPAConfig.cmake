# -*- mode: cmake; -*-

set(ENABLE_SDPA_GPI No)
set(ENABLE_GPI_SPACE Yes)
set(GPI_PRIV_DIR /tmp)
set(WE_PRECOMPILE OFF)
set(CMAKE_BUILD_TYPE Release)

set(WITH_FAKE_PC On)

set(SEDA_FHGLOG On)
set(SEDA_LOG4CPP Off)
set(SEDA_COMM  Off)

set(BOOST_ROOT /opt/boost/1.45/gcc)
set(BOOST_FILESYSTEM_VERSION 3)

set(SMC_HOME /opt/smc/5.0.0)


set(APPLICATION_LATEST /home/projects/sdpa/external_software/latest/application)
# set(BUILDROBOT_LATEST  /home/projects/sdpa/external_software/latest/buildrobot )
set(EDITOR_LATEST      /home/projects/sdpa/external_software/latest/editor)
set(FHGCOM_LATEST      /home/projects/sdpa/external_software/latest/fhgcom)
set(FHGLOG_LATEST      /home/projects/sdpa/external_software/latest/fhglog)
set(FUSE_LATEST        /home/projects/sdpa/external_software/latest/fuse)
set(FVM_LATEST         /home/projects/sdpa/external_software/latest/fvm-pc)
set(GPI-SPACE_LATEST   /home/projects/sdpa/external_software/latest/gpi-space)
# set(MAIN_LATEST        /home/projects/sdpa/external_software/latest/)
set(MMGR_LATEST        /home/projects/sdpa/external_software/latest/mmgr)
# set(MODULES_LATEST     /home/projects/sdpa/external_software/latest/)
set(MONITOR_LATEST     /home/projects/sdpa/external_software/latest/monitor)
set(PLAYGROUND_LATEST  /home/projects/sdpa/external_software/latest/playground)
set(REWRITE_LATEST     /home/projects/sdpa/external_software/latest/rewrite)
set(SDPA_LATEST        /home/projects/sdpa/external_software/latest/sdpa)
set(SDPA-GPI_LATEST    /home/projects/sdpa/external_software/latest/sdpa-gpi)
set(SDPA-GUI_LATEST    /home/projects/sdpa/external_software/latest/sdpa-gui)
set(SEDA_LATEST        /home/projects/sdpa/external_software/latest/seda)
set(UTIL_LATEST        /home/projects/sdpa/external_software/latest/util)
set(WE_LATEST          /home/projects/sdpa/external_software/latest/we)
set(XML_LATEST         /home/projects/sdpa/external_software/latest/xml)

set(CTEST_UPDATE_OPTIONS "--username SVN_hpc_bitter --password nuphFan0")

