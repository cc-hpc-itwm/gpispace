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

install(CODE "
  file(GLOB_RECURSE PLUGINS
         \"\${CMAKE_INSTALL_PREFIX}/libexec/fhg/plugins/*.so\")
  execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bundle.sh
                          -p \"\${CMAKE_INSTALL_PREFIX}\"
                          -d
                          -x \"libgcc_s\"
                          -x \"libc.so\"
                          -x \"libdl.so\"
                          -x \"libstdc++.so\"
                          -x \"libpthread.so\"
                          -x \"libgthread.so\"
                          -x \"libglib.so\"
                          -x \"libfontconfig.so\"
                          -x \"libibverbs.so\"
                          -w \"libQt.*.so\"
                          -w \"libboost.*.so\"
                          -w \"libhwloc.*.so\"
                          -v
                           ${APPS} \${PLUGINS}
  )

  " COMPONENT dependencies)
