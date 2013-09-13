# -*- mode: cmake; -*-

set (simple_targets
     we-exec
     we-eval
     pnetget
     pnetput
     pnet2dot
     pneti
     pnetsc
     pnetlt
    )

foreach (target ${simple_targets})

  add_executable (${target} "${target}.cpp")

  target_link_libraries(${target} fhg-revision)
  target_link_libraries(${target} pnet)
  target_link_libraries(${target} we-dev)
  target_link_libraries(${target} ${UTIL_LIBRARIES})
  target_link_libraries(${target} ${FhgLog_LIBRARY})
  target_link_libraries(${target} ${Boost_LIBRARIES})
  target_link_libraries(${target} pthread)
  target_link_libraries(${target} ${CMAKE_DL_LIBS})

  install (TARGETS ${target}
           RUNTIME
           DESTINATION bin
           COMPONENT runtime
          )

  if (ENABLE_STATIC_BINARIES)

    add_executable(${target}.static "${target}.cpp")

    target_link_libraries(${target}.static pnet)
    target_link_libraries(${target}.static ${FhgLog_LIBRARY})
    target_link_libraries(${target}.static ${Boost_LIBRARIES})
    target_link_libraries(${target}.static pthread)
    target_link_libraries(${target}.static ${CMAKE_DL_LIBS})
    target_link_libraries(${target}.static fhg-revision)

    set_target_properties(${target}.static PROPERTIES LINK_FLAGS "-static")

    install (TARGETS ${target}.static
             RUNTIME
             DESTINATION bin
             COMPONENT runtime
            )
  endif (ENABLE_STATIC_BINARIES)
endforeach (target ${simple_targets})

if (ENABLE_PNETOPT)
  add_subdirectory(pnetopt)
endif ()

if (ENABLE_PNETV)
  add_subdirectory(pnetv)
endif ()
