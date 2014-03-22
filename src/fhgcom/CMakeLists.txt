include_directories (${FhgCom_INCLUDE_DIR})

include_directories (${FhgLog_INCLUDE_DIR})

include_directories (${UTIL_INCLUDE_DIR})

set (hdrs
  message.hpp

  session.hpp
  basic_session_manager.hpp
  session_manager.hpp
  connection.hpp
  peer.hpp
  peer_info.hpp

  tcp_server.hpp
  tcp_client.hpp
  io_service_pool.hpp

  header.hpp
)

set (srcs
  session.cpp
  peer.cpp
  connection.cpp

  tcp_server.cpp
  tcp_client.cpp
  io_service_pool.cpp

  header.cpp
)

ADD_LIBRARY(fhgcom ${hdrs} ${srcs})
ADD_LIBRARY(fhgcom-shared SHARED ${hdrs} ${srcs})
#target_link_libraries (fhgcom-shared ${FhgLog_LIBRARY})
set_target_properties(fhgcom-shared PROPERTIES OUTPUT_NAME fhgcom)

  add_subdirectory (tests)
