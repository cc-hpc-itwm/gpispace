# bundle up
list(APPEND APPS "\${CMAKE_INSTALL_PREFIX}/bin/sdpac")

if (TARGET sdpa-gui)
  list(APPEND APPS "\${CMAKE_INSTALL_PREFIX}/bin/sdpa-gui")
endif()
if (TARGET gpish)
  list(APPEND APPS "\${CMAKE_INSTALL_PREFIX}/bin/gpish")
endif()
if (TARGET gpi-space)
  list(APPEND APPS "\${CMAKE_INSTALL_PREFIX}/bin/gpi-space")
endif()
if (TARGET pnete)
  list(APPEND APPS "\${CMAKE_INSTALL_PREFIX}/bin/pnete")
endif()

list(APPEND libSearchDirs $ENV{BOOST_ROOT}/lib/)
list(APPEND libSearchDirs $ENV{HWLOC_HOME}/lib/)
list(APPEND libSearchDirs ${QT_LIBRARY_DIR}/)
list(APPEND libSearchDirs $ENV{GRAPHVIZ_HOME}/lib/)
#list(APPEND libSearchDirs "\${CMAKE_INSTALL_PREFIX}/lib/")
#list(APPEND libSearchDirs "\${CMAKE_INSTALL_PREFIX}/libexec/")
#list(APPEND libSearchDirs /lib /lib64 /usr/lib /usr/lib64)

foreach(app ${APPS})
  include(InstallRequiredSystemLibraries)
  install(CODE "
  file(GLOB_RECURSE SHARED_LIBS
         \"\${CMAKE_INSTALL_PREFIX}/*.so\")
  include(BundleUtilities)
  fixup_bundle(\"${app}\" \"\${SHARED_LIBS}\" \"${libSearchDirs}\")
  " COMPONENT runtime)
endforeach()
