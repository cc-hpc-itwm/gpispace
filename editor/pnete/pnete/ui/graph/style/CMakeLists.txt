#bernd.loerwald@itwm.fraunhofer.de

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

add_library("${LIB_PREFIX}style" ${style_SRCS} ${style_SRCS_MOC})

add_executable (test_pnete_cached_predicates test_cached_predicates.cpp)
add_test ("test_pnete_cached_predicates" test_pnete_cached_predicates)
