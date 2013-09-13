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
if (TARGET gspcmond)
  list(APPEND APPS "\${CMAKE_INSTALL_PREFIX}/bin/gspcmond")
endif()
if (TARGET gspcmonc)
  list(APPEND APPS "\${CMAKE_INSTALL_PREFIX}/bin/gspcmonc")
endif()
if (TARGET fhgkernel)
  list(APPEND APPS "\${CMAKE_INSTALL_PREFIX}/bin/fhgkernel")
endif()
if (TARGET gspcnet-req)
  list(APPEND APPS "\${CMAKE_INSTALL_PREFIX}/bin/gspcnet-req")
endif()
if (TARGET gspcnet-ping)
  list(APPEND APPS "\${CMAKE_INSTALL_PREFIX}/bin/gspcnet-ping")
endif()

install(CODE "
  file(GLOB_RECURSE PLUGINS
         \"\${CMAKE_INSTALL_PREFIX}/libexec/fhg/plugins/*.so\")
  execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bundle.sh
                          -p \"\${CMAKE_INSTALL_PREFIX}\"
                          -d
                          -f
                          -L \"\${CMAKE_INSTALL_PREFIX}/lib\"
                          -x libibverbs.*
                          -x libxcb.*
                          -x libSM.*
                          -x libc.so.*
                          -x libstdc++.so.*
                          -x libgcc_s.so.*
                          -x libz.so.*
                          -x libm.so.*
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
                  RESULT_VARIABLE __res
                  ERROR_VARIABLE __err
  )
  if (NOT \${__res} EQUAL 0)
     message(FATAL_ERROR \"Could not bundle dependencies: \${__err}\")
  endif()
  " COMPONENT dependencies)
