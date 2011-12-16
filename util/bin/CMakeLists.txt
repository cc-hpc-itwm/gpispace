add_executable(fhgcfg fhgcfg.cpp)
target_link_libraries(fhgcfg ${Boost_LIBRARIES})

add_executable(fhgmodinfo fhg-modinfo.cpp)
target_link_libraries(fhgmodinfo fhg-plugin)

add_executable(fhgkernel fhgkernel.cpp)
target_link_libraries(fhgkernel fhg-plugin)

find_package(FhgLog REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})

install (TARGETS fhgcfg fhgmodinfo fhgkernel
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
