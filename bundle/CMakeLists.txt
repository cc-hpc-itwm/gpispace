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

foreach(app ${APPS})
  include(InstallRequiredSystemLibraries)
  install(CODE "
  file(GLOB_RECURSE PLUGINS
         \"\${CMAKE_INSTALL_PREFIX}/libexec/plugins/*.so\")
  message(STATUS \"plugins='\${PLUGINS}'\")
  include(BundleUtilities)
  fixup_bundle(\"${app}\" \"\${PLUGINS}\" \"${libSearchDirs}\")
  " COMPONENT runtime)
endforeach()
