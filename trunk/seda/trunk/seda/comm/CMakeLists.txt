# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set (hdrs
  comm.hpp
  ConnectionFactory.hpp
  Connection.hpp
  ConnectionListener.hpp
  ConnectionStrategy.hpp
  Decodeable.hpp
  delivery_service.hpp
  Encodeable.hpp
  Encoder.hpp
  exception.hpp
  Locator.hpp
  LocatorPersister.hpp
  option.hpp
  SedaCommError.hpp
  SedaMessage.hpp
  Serialization.hpp
  ServiceThread.hpp
  UDPConnection.hpp
  )

set (srcs
  comm.cpp
  Connection.cpp
  ConnectionFactory.cpp
  ConnectionStrategy.cpp
  LocatorPersister.cpp
  SedaMessage.cpp
  UDPConnection.cpp
  )

ADD_LIBRARY(seda-comm ${hdrs} ${srcs})
target_link_libraries (seda-comm ${ZLIB_LIBRARIES})

install(FILES ${hdrs}
  DESTINATION include/seda/comm
  COMPONENT headers)
install(TARGETS seda-comm
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries)
