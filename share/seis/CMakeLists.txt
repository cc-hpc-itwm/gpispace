
extended_add_library (NAME determine_size
  TYPE SHARED
  SOURCES "determine_size.cpp"
          "swap_bytes.cpp"
  INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/share/seis"
  INSTALL
  INSTALL_DESTINATION "libexec/gspc"
)
extended_add_library (NAME do_load
  TYPE SHARED
  SOURCES "do_load.cpp"
          "swap_bytes.cpp"
  INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/share/seis"
  INSTALL
  INSTALL_DESTINATION "libexec/gspc"
)
extended_add_library (NAME do_write
  TYPE SHARED
  SOURCES "do_write.cpp"
          "swap_bytes.cpp"
  INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/share/seis"
  INSTALL
  INSTALL_DESTINATION "libexec/gspc"
)

install (FILES do_load.hpp determine_size.hpp do_write.hpp
  DESTINATION include/seis
)
