#bernd.loerwald@itwm.fraunhofer.de

include_directories (SYSTEM ${GRAPHVIZ_INCLUDE_DIR})

add_library ("${LIB_PREFIX}graphviz-wrapper" graphviz.cpp)
target_link_libraries ("${LIB_PREFIX}graphviz-wrapper" ${GRAPHVIZ_LIBRARIES})
