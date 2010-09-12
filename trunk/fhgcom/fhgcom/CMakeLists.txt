# -*- mode: cmake; -*-

add_subdirectory(util)
add_subdirectory(kvs)

set (hdrs
  memory.hpp
  message.hpp
  basic_session.hpp
  session.hpp
  basic_session_manager.hpp
  session_manager.hpp

  tcp_server.hpp
  tcp_client.hpp
  io_service_pool.hpp
)

set (srcs
  message.cpp
  session.cpp

  tcp_server.cpp
  tcp_client.cpp
  io_service_pool.cpp
)

if (NO_IMPLICIT_TEMPLATES)
  set (srcs ${srcs} template.cpp)
endif (NO_IMPLICIT_TEMPLATES)

ADD_LIBRARY(fhgcom ${hdrs} ${srcs})
ADD_LIBRARY(fhgcom-shared SHARED ${hdrs} ${srcs})
set_target_properties(fhgcom-shared PROPERTIES OUTPUT_NAME fhgcom)

install(FILES ${hdrs} DESTINATION include/fhgcom)
install(TARGETS fhgcom DESTINATION lib EXPORT ${PROJECT_NAME}Exports)
install(TARGETS fhgcom-shared DESTINATION lib EXPORT ${PROJECT_NAME}Exports)

