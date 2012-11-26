#bernd.loerwald@itwm.fraunhofer.de

include(${QT_USE_FILE})

set(style_SRCS
  association.cpp
  cap.cpp
  raster.cpp
  size.cpp
  fallback.cpp
  predicate.cpp
  type.cpp
)

set(style_HDRS
  association.hpp
  cap.hpp
  raster.hpp
  size.hpp
  style.hpp
  style.fwd.hpp
  fallback.hpp
  store.hpp
  predicate.hpp
  type.hpp
  mode.hpp
)

set(style_UIS
)

set(style_TOMOC
)

#for moced files to get their includes.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
QT4_WRAP_UI(style_UIS_H ${style_UIS})
QT4_WRAP_CPP(style_SRCS_MOC ${style_TOMOC} ${style_UIS_H})

qt4_automoc(${style_SRCS_MOC})
add_library("${LIB_PREFIX}style" ${style_SRCS} ${style_SRCS_MOC})
add_dependencies("${LIB_PREFIX}style" gen_headerlist)

add_executable (test_pnete_cached_predicates test_cached_predicates.cpp)
add_test ("test_pnete_cached_predicates" test_pnete_cached_predicates)
