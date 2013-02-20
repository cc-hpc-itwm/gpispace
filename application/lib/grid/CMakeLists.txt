set (files
  nth.xpnet
  size.xpnet
  type.xpnet
)

install (FILES ${files}
  DESTINATION ${PNETC_LIB_DIR}/grid
  COMPONENT examples
  )

add_subdirectory (gen_grid_points)
