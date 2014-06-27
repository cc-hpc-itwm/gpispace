# bernd.loerwald@itwm.fraunhofer.de

add_library (rpc client.cpp server.cpp)
target_link_libraries (rpc fhg-util net ${Boost_SERIALIZATION_LIBRARY})
