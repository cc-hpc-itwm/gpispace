# -*- mode: cmake; -*-
include (${QT_USE_FILE})

set ( graphviz_wrapper_SRCS
      graphviz.cpp
    )

add_library (graphviz-wrapper ${graphviz_wrapper_SRCS})
target_link_libraries (graphviz-wrapper ${GRAPHVIZ_LIBRARIES})
