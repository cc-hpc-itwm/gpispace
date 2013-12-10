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

add_executable(pnetc pnetc.cpp)
target_link_libraries(pnetc fhg-revision)
target_link_libraries(pnetc pnet)
target_link_libraries(pnetc ${Boost_FILESYSTEM_LIBRARY} ${Boost_LIBRARIES})

install (TARGETS pnetc RUNTIME DESTINATION bin COMPONENT runtime)
