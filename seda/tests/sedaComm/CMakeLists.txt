# -*- mode: cmake; -*-

set(sedaCommTest_src main.cpp
  EncoderTest.cpp
  UDPConnectionTest.cpp
)
set(msg_listener_src msg_listener.cpp)
set(msg_sender_src   msg_sender.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(${${PROJECT_NAME}_SOURCE_DIR}/etc/log4j.properties ${CMAKE_CURRENT_BINARY_DIR}/log4j.properties COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testsconfig.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/testsconfig.hpp)

add_executable(sedaCommTestsuite ${sedaCommTest_src})
add_executable(msg_listener ${msg_listener_src})
target_link_libraries(msg_listener pthread)

add_executable(msg_sender ${msg_sender_src})
target_link_libraries(msg_sender pthread)

add_test(sedaCommTestsuite ${CMAKE_CURRENT_BINARY_DIR}/sedaCommTestsuite)
target_link_libraries(sedaCommTestsuite seda)
target_link_libraries(sedaCommTestsuite seda-comm)
target_link_libraries(sedaCommTestsuite ${CppUnit_LIBRARY})
target_link_libraries(sedaCommTestsuite ${CMAKE_DL_LIBS})
target_link_libraries(sedaCommTestsuite pthread)
target_link_libraries(msg_listener seda seda-comm)
target_link_libraries(msg_sender seda seda-comm)

set(test_srcs test_udp.cpp)
add_executable(test_udp ${test_srcs})
add_test(test_sedaComm_udp ${CMAKE_CURRENT_BINARY_DIR}/test_udp)
target_link_libraries(test_udp seda seda-comm)
target_link_libraries(test_udp pthread)

add_executable(test_retrans test_retransmission.cpp)
add_test(test_sedaComm_retrans ${CMAKE_CURRENT_BINARY_DIR}/test_retrans)
target_link_libraries(test_retrans seda seda-comm)
target_link_libraries(test_retrans pthread)

set(test_srcs test_persister.cpp)
add_executable(test_persister ${test_srcs})
add_test(test_sedaComm_persister ${CMAKE_CURRENT_BINARY_DIR}/test_persister)
target_link_libraries(test_persister seda seda-comm)
target_link_libraries(test_persister pthread)

set(test_srcs test_message_size.cpp)
add_executable(test_message_size ${test_srcs})
add_test(test_sedaComm_message_size ${CMAKE_CURRENT_BINARY_DIR}/test_message_size)
target_link_libraries(test_message_size seda seda-comm)
target_link_libraries(test_message_size pthread)
