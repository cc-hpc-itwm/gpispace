# -*- mode: cmake; -*-

add_executable(we-exec "we-exec.cpp")
target_link_libraries(we-exec ${FhgLog_LIBRARY})
target_link_libraries(we-exec ${Boost_LIBRARIES})
target_link_libraries(we-exec pthread)
target_link_libraries(we-exec dl)
install (TARGETS we-exec
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

add_executable(loader loader.cpp)
target_link_libraries(loader ${FhgLog_LIBRARY})
target_link_libraries(loader ${Boost_LIBRARIES})
target_link_libraries(loader pthread)
target_link_libraries(loader dl)
install (TARGETS loader
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

add_executable(pnet2dot pnet2dot.cpp)
target_link_libraries(pnet2dot ${Boost_LIBRARIES})
install (TARGETS pnet2dot
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

if (ENABLE_STATIC_BINARIES)
  add_executable(pnet2dot.static pnet2dot.cpp)
  target_link_libraries(pnet2dot.static ${FhgLog_LIBRARY})
  target_link_libraries(pnet2dot.static ${Boost_LIBRARIES})
  target_link_libraries(pnet2dot.static pthread)
  target_link_libraries(pnet2dot.static dl)
  set_target_properties(pnet2dot.static PROPERTIES LINK_FLAGS "-static")
  install (TARGETS pnet2dot.static
    RUNTIME
    DESTINATION bin
    COMPONENT applications)
endif (ENABLE_STATIC_BINARIES)
