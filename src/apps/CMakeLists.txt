include (fhg_executable_macros)

set (simple_targets
     we-exec
     we-eval
     pnetget
     pnetput
     pnet2dot
     pneti
    )

foreach (target ${simple_targets})

  add_executable (${target} "${target}.cpp")

  target_link_libraries(${target} fhg-revision)
  target_link_libraries(${target} pnet)
  target_link_libraries(${target} we-dev)
  target_link_libraries(${target} ${UTIL_LIBRARIES})
  target_link_libraries(${target} ${FhgLog_LIBRARY})
  target_link_libraries(${target} ${Boost_FILESYSTEM_LIBRARY}) #we-eval, we-exec only
  target_link_libraries(${target} ${Boost_LIBRARIES})
  target_link_libraries(${target} pthread)
  target_link_libraries(${target} ${CMAKE_DL_LIBS})

  install (TARGETS ${target}
           RUNTIME
           DESTINATION bin
           COMPONENT runtime
          )
endforeach (target ${simple_targets})

fhg_add_runtime_executable (
  pnetsc.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnetlt.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnetc.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
)
