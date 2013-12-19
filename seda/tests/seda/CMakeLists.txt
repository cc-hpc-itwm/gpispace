# -*- mode: cmake; -*-

set(sedatest_src main.cpp
   SedaStageTest.cpp  SedaStageTest.hpp
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(sedatestsuite ${sedatest_src})
add_test(sedatestsuite ${CMAKE_CURRENT_BINARY_DIR}/sedatestsuite)
target_link_libraries(sedatestsuite ${CppUnit_LIBRARY})
target_link_libraries (sedatestsuite ${Boost_THREAD_LIBRARY})
target_link_libraries (sedatestsuite ${Boost_SYSTEM_LIBRARY})
if( NOT WIN32 )
  target_link_libraries(sedatestsuite pthread)
endif( NOT WIN32 )
