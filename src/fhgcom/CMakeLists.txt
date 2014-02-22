set (FHG_COM_NS_UUID "c9fe00cb-d9f7-432e-9235-66b7929b6e2a" CACHE STRING "The unique namespace uuid for this installation")

include_directories (${FhgCom_INCLUDE_DIR})

find_package (ZLIB REQUIRED QUIET)
include_directories (SYSTEM ${ZLIB_INCLUDE_DIRS})

include_directories (${FhgLog_INCLUDE_DIR})

include_directories (${UTIL_INCLUDE_DIR})

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/ns_uuid.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/ns_uuid.cpp)

add_subdirectory(util)
add_subdirectory(kvs)

set (hdrs
  memory.hpp
  message.hpp
  message_io.hpp

  basic_session.hpp
  session.hpp
  basic_session_manager.hpp
  session_manager.hpp
  connection.hpp
  peer.hpp
  peer_io.hpp
  peer_info.hpp

  tcp_server.hpp
  tcp_client.hpp
  io_service_pool.hpp

  ns_uuid.hpp
  header.hpp
)

set (srcs
  session.cpp
  peer.cpp
  connection.cpp

  tcp_server.cpp
  tcp_client.cpp
  io_service_pool.cpp

  ns_uuid.cpp
  header.cpp
)

ADD_LIBRARY(fhgcom ${hdrs} ${srcs})
ADD_LIBRARY(fhgcom-shared SHARED ${hdrs} ${srcs})
target_link_libraries (fhgcom ${ZLIB_LIBRARIES})
#target_link_libraries (fhgcom-shared ${ZLIB_LIBRARIES} ${FhgLog_LIBRARY})
set_target_properties(fhgcom-shared PROPERTIES OUTPUT_NAME fhgcom)

add_subdirectory (bin)

  add_subdirectory (tests)
