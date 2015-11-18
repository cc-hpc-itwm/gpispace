extended_add_library (NAME vmem
  NAMESPACE GPISpace
  SOURCES "gaspi_context.cpp"
  LIBRARIES GASPI::static
)
#! \todo move into extended_add_library
target_include_directories (GPISpace-vmem
  SYSTEM PRIVATE "${GASPI_INCLUDE_DIR}"
)
