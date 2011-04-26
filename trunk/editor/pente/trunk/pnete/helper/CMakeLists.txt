include(${QT_USE_FILE})

set(helper_SRCS
  XMLQuery.cpp
)

set(helper_HDRS
  XMLQuery.hpp
)

#for moced files to get their includes.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_library (helper ${helper_SRCS})
