add_subdirectory(util)

add_custom_target (generate_revision_cpp ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/../gen-revision-cpp ${CMAKE_CURRENT_SOURCE_DIR}/revision.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "generating revision.cpp..."
    SOURCES ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp
    )

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/revision.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp @ONLY)

add_library (fhg-revision ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp)
add_dependencies (fhg-revision generate_revision_cpp)

install (FILES revision.hpp
  DESTINATION include/fhg
  COMPONENT headers
  )
