# -*- mode: cmake; -*-

set (simple_targets
     we-exec
     loader
     pnetget
     pnetput
     pnet2dot
    )

foreach (target ${simple_targets})

  add_executable (${target} "${target}.cpp")

  target_link_libraries(${target} ${WE_LIBRARIES})
  target_link_libraries(${target} ${FhgLog_LIBRARY})
  target_link_libraries(${target} ${Boost_LIBRARIES})
  target_link_libraries(${target} pthread)
  target_link_libraries(${target} dl)

  install (TARGETS ${target}
           RUNTIME
           DESTINATION bin
           COMPONENT runtime
          )

  if (ENABLE_STATIC_BINARIES)

    add_executable(${target}.static "${target}.cpp")

    target_link_libraries(${target}.static ${WE_LIBRARIES})
    target_link_libraries(${target}.static ${FhgLog_LIBRARY})
    target_link_libraries(${target}.static ${Boost_LIBRARIES})
    target_link_libraries(${target}.static pthread)
    target_link_libraries(${target}.static dl)

    set_target_properties(${target}.static PROPERTIES LINK_FLAGS "-static")

    install (TARGETS ${target}.static
             RUNTIME
             DESTINATION bin
             COMPONENT runtime
            )
  endif (ENABLE_STATIC_BINARIES)
endforeach (target ${simple_targets})

add_subdirectory(pnetv)
