# -*- mode: cmake; -*-

add_executable(we-exec "we-exec.cpp")
target_link_libraries(we-exec ${FhgLog_LIBRARY})
target_link_libraries(we-exec ${Boost_LIBRARIES})
target_link_libraries(we-exec dl)
target_link_libraries(we-exec pthread)
install (TARGETS we-exec
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

add_executable(loader loader.cpp)
target_link_libraries(loader ${FhgLog_LIBRARY})
target_link_libraries(loader ${Boost_LIBRARIES})
target_link_libraries(loader dl)
target_link_libraries(loader pthread)
install (TARGETS loader
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

add_executable(pnet2dot pnet2dot.cpp)
target_link_libraries(pnet2dot ${Boost_LIBRARIES})

add_executable(pnet2dot.static pnet2dot.cpp)
target_link_libraries(pnet2dot.static ${Boost_LIBRARIES})
set_target_properties(pnet2dot.static PROPERTIES LINK_FLAGS "-static")

install (TARGETS pnet2dot pnet2dot.static
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
