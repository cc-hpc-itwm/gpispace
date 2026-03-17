find_package (OpenSSL REQUIRED)

gspc_add_library (NAME gspc_com
  TYPE STATIC
  SOURCES
    "address.cpp"
    "channel.cpp"
    "connection.cpp"
    "header.cpp"
    "message.cpp"
    "peer.cpp"
    "peer_info.cpp"
  INCLUDE_DIRECTORIES
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  LIBRARIES
    PUBLIC gspc_util
    PUBLIC Boost::system
    PUBLIC Boost::thread
    PUBLIC fmt
    PUBLIC GPISpace-header-only
    PUBLIC gspc_assert
    PRIVATE OpenSSL::SSL
    PRIVATE OpenSSL::Crypto
)

# Backwards compatibility alias
add_library (fhgcom ALIAS gspc_com)
