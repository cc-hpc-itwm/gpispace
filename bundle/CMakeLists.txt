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
list(APPEND libSearchDirs \${CMAKE_INSTALL_PREFIX}/lib/)

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

# move libraries to lib
install(CODE "
  file(GLOB LIBS RELATIVE \"\${CMAKE_INSTALL_PREFIX}/bin\"
	 \"\${CMAKE_INSTALL_PREFIX}/bin/*.so\"
	 \"\${CMAKE_INSTALL_PREFIX}/bin/*.so.*\")
  message(STATUS \"moving libraries \(\${LIBS}\) from bin to lib\")
  foreach(f \${LIBS})
    file(RENAME \"\${CMAKE_INSTALL_PREFIX}/bin/\${f}\" \"\${CMAKE_INSTALL_PREFIX}/lib/\${f}\")
  endforeach()
  
  " COMPONENT runtime)
