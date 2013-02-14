#bernd.loerwald@itwm.fraunhofer.de

add_library ("${LIB_PREFIX}graphviz-wrapper" graphviz.cpp)
target_link_libraries ("${LIB_PREFIX}graphviz-wrapper" ${GRAPHVIZ_LIBRARIES})
