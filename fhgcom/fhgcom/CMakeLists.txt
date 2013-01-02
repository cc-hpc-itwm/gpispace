# -*- mode: cmake; -*-

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
#target_link_libraries (fhgcom-shared ${ZLIB_LIBRARIES} ${Boost_LIBRARIES} ${FhgLog_LIBRARY})
set_target_properties(fhgcom-shared PROPERTIES OUTPUT_NAME fhgcom)

install(FILES ${hdrs}
  DESTINATION include/fhgcom
  COMPONENT headers)

install(TARGETS fhgcom
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries)

install(TARGETS fhgcom-shared
  LIBRARY DESTINATION lib
  COMPONENT libraries)
