# -*- mode: cmake; -*-
## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.37.0" "1.37" "1.36.0" "1.36")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS ON)
include(FindBoost)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system serialization)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})
link_libraries(${Boost_LIBRARIES})

set (hdrs RemoteAppender.hpp LogServer.hpp Serialization.hpp)
set (srcs RemoteAppender.cpp LogServer.cpp)

ADD_LIBRARY(fhglog_remote ${hdrs} ${srcs})
add_executable(fhglog-server fhglog-server.cpp)
add_executable(fhglog-client fhglog-client.cpp)

target_link_libraries(fhglog-server fhglog fhglog_remote)
target_link_libraries(fhglog-client fhglog fhglog_remote)

install(FILES ${hdrs} DESTINATION include/fhglog/remote)
install(TARGETS fhglog_remote DESTINATION lib)
install(TARGETS fhglog-server DESTINATION bin)
install(TARGETS fhglog-client DESTINATION bin)
