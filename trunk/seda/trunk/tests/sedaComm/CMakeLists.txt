# -*- mode: cmake; -*-

set(sedaCommTest_src main.cpp
  EncoderTest.cpp
  ZMQConnectionTest.cpp
)
set(msg_listener_src msg_listener.cpp)
set(msg_sender_src   msg_sender.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(${${PROJECT_NAME}_SOURCE_DIR}/etc/log4j.properties ${CMAKE_CURRENT_BINARY_DIR}/log4j.properties COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testsconfig.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/testsconfig.hpp)

add_executable(sedaCommTestsuite ${sedaCommTest_src})
add_executable(msg_listener ${msg_listener_src})
add_executable(msg_sender ${msg_sender_src})
add_test(sedaCommTestsuite ${CMAKE_CURRENT_BINARY_DIR}/sedaCommTestsuite)
target_link_libraries(sedaCommTestsuite seda sedaComm)
target_link_libraries(msg_listener seda sedaComm)
target_link_libraries(msg_sender seda sedaComm)
