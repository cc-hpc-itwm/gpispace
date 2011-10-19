# -*- mode: cmake; -*-
include (${QT_USE_FILE})

set ( graphviz_wrapper_SRCS
      graphviz.cpp
    )

add_library ("${LIB_PREFIX}graphviz-wrapper" ${graphviz_wrapper_SRCS})
target_link_libraries ("${LIB_PREFIX}graphviz-wrapper" ${GRAPHVIZ_LIBRARIES})
