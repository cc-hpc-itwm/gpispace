# bundle up

set (APPS)

macro (BUNDLE_APPS)
  foreach (app ${ARGN})
      get_target_property (_loc ${app} LOCATION)
      list(APPEND APPS "${_loc}")
      unset (_loc)
  endforeach()
endmacro()

bundle_apps(
  agent
  drts-kernel
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
  file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/lib/graphviz\")
  file(COPY \"${CMAKE_CURRENT_SOURCE_DIR}/graphviz/config6\"
       DESTINATION \"\${CMAKE_INSTALL_PREFIX}/lib/graphviz\")
  file(GLOB GRAPHVIZ_PLUGIN_CORE \"${GRAPHVIZ_PLUGIN_DIR}/libgvplugin_core.so*\")
  file(GLOB GRAPHVIZ_PLUGIN_DOT \"${GRAPHVIZ_PLUGIN_DIR}/libgvplugin_dot_layout.so*\")
  file(COPY \${GRAPHVIZ_PLUGIN_CORE} DESTINATION \"\${CMAKE_INSTALL_PREFIX}/lib/graphviz\")
  file(COPY \${GRAPHVIZ_PLUGIN_DOT} DESTINATION \"\${CMAKE_INSTALL_PREFIX}/lib/graphviz\")
  file(COPY \"${BOOST_ROOT}/\" DESTINATION \"\${CMAKE_INSTALL_PREFIX}/external/boost\")

  file(GLOB_RECURSE PLUGINS
         \"\${CMAKE_INSTALL_PREFIX}/libexec/fhg/plugins/*.so\")
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/sdpa/bin/pbs/bundle.sh
                          -p \"\${CMAKE_INSTALL_PREFIX}\"
                          -d
                          -L \"\${CMAKE_INSTALL_PREFIX}/lib\"
                          -x libibverbs.*
                          -x libxcb.*
                          -x libSM.*
                          -x libc.so.*
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
                          -x libreadline.*
                          -x libboost*.*
                           ${APPS} \${PLUGINS}
                  RESULT_VARIABLE __res
                  ERROR_VARIABLE __err
  )
  if (NOT \${__res} EQUAL 0)
     message(FATAL_ERROR \"Could not bundle dependencies: \${__err}\")
  endif()
  " )
