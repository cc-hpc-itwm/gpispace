
add_subdirectory (generate)

set (xml
   diff.xml.in
)

# include(pnet_macros)

# pnet_compile(parsu_diff diff.xml.in
#   INCLUDES generate
#            ${CMAKE_SOURCE_DIR}/share/lib
#   GENERATE parsu_diff
#   CXXFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/../util
#            -I${CMAKE_SOURCE_DIR}/fhglog
#            -I${CMAKE_BINARY_DIR}/fhglog
#            -O3
#   BUILD
#   INSTALL libexec/apps/parsu/diff
# )
