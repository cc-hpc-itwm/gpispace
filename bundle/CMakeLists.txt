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
                          -v
                          -x libibverbs.*
                          -x libxcb.*
                          -x libstdc++.*
                          -x libSM.*
                          -x libgcc_s.so
                          -x libc.so.*
                          -x librt.*
                          -x libfont.*
                          -x libfreetype.*
                          -x libaudio.*
                          -x libICE.*
                          -x libglib.*
                          -x libgobject.*
                          -x libdl.*
                          -x libX.*so
                          -x libpthread.*
                          -x libgthread.*
                           ${APPS} \${PLUGINS}
  )

  " COMPONENT dependencies)
