set (files
  Makefile
  gen_grid_points.xpnet
)

install (FILES ${files}
  DESTINATION ${PNETC_EXAMPLES_DIR}/grid/gen_grid_points
  COMPONENT examples
)
