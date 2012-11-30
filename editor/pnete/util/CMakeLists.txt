#bernd.loerwald@itwm.fraunhofer.de

set ( graphviz_wrapper_SRCS
      graphviz.cpp
    )

add_subdirectory (qt)

add_library ("${LIB_PREFIX}graphviz-wrapper" ${graphviz_wrapper_SRCS})
target_link_libraries ("${LIB_PREFIX}graphviz-wrapper" ${GRAPHVIZ_LIBRARIES})
