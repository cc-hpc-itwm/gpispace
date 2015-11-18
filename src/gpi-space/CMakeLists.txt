add_subdirectory(gpi)
add_subdirectory(pc)
add_subdirectory (tests)

extended_add_library (NAME gpi_type_check
  SOURCES "types.cpp"
  LIBRARIES gpi-space-gpiapi
)
