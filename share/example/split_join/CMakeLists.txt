set (SPLIT_JOIN_SPLIT_VERSIONS
  join_after_split
  parallel_split_join
)

set (SPLIT_JOIN_REENTRANT_VERSIONS
  not_reentrant
  reentrant.sequential
)

foreach (SPLIT_VERSION ${SPLIT_JOIN_SPLIT_VERSIONS})
  foreach (REENTRANT_VERSION ${SPLIT_JOIN_REENTRANT_VERSIONS})
    configure_file (
      ${CMAKE_CURRENT_SOURCE_DIR}/split_join.${REENTRANT_VERSION}.xpnet.in
      ${CMAKE_CURRENT_BINARY_DIR}/split_join.${SPLIT_VERSION}.${REENTRANT_VERSION}.xpnet
      @ONLY
    )
  endforeach()

  configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/split.${SPLIT_VERSION}.xpnet
    ${CMAKE_CURRENT_BINARY_DIR}/split.${SPLIT_VERSION}.xpnet
    COPYONLY
  )
endforeach()

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/join.xpnet
  ${CMAKE_CURRENT_BINARY_DIR}/join.xpnet
  COPYONLY
)

add_subdirectory (test)
