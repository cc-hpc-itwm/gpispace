# -*- mode: cmake; -*-
set (hdrs RemoteAppender.hpp LogServer.hpp Serialization.hpp)
set (srcs RemoteAppender.cpp LogServer.cpp)

ADD_LIBRARY(fhglog_remote ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/fhglog/remote)
install(TARGETS fhglog_remote DESTINATION lib)
