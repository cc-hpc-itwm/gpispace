# bundle up

install (PROGRAMS bundle.sh
  DESTINATION libexec/sdpa/scripts
)

if (INSTALL_AS_BUNDLE)

set (APPS)

macro (BUNDLE_APPS)
  foreach (app ${ARGN})
    if (TARGET ${app})
      get_target_property (_loc ${app} LOCATION)
      list(APPEND APPS "${_loc}")
      unset (_loc)
    endif()
  endforeach()
endmacro()

bundle_apps(
  agent
  drts-kernel
  fhgcfg
  fhgkvsc
  fhgkvsd
  fhglog-dump
  fhglogc
  fhglogd
  gpi-space
  gpish
  orchestrator
  pnet2dot
  pnetc
  pnetget
  pnetput
  pnetv
  sdpac
  we-exec
  sdpa-gui
  gspcmonc
  pnete
)

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
  " )

endif()
