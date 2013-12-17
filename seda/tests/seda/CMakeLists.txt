# -*- mode: cmake; -*-

set(sedatest_src main.cpp
   EventQueueTest.cpp EventQueueTest.hpp
   SedaStageTest.cpp  SedaStageTest.hpp
   AccumulateStrategyTest.cpp  AccumulateStrategyTest.hpp
   TimerTest.cpp TimerTest.hpp
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testsconfig.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/testsconfig.hpp)

add_executable(sedatestsuite ${sedatest_src})
add_test(sedatestsuite ${CMAKE_CURRENT_BINARY_DIR}/sedatestsuite)
target_link_libraries(sedatestsuite seda)
target_link_libraries(sedatestsuite ${CppUnit_LIBRARY})
target_link_libraries (sedatestsuite ${Boost_THREAD_LIBRARY})
if( NOT WIN32 )
  target_link_libraries(sedatestsuite pthread)
endif( NOT WIN32 )
