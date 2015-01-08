
include_directories (${CMAKE_SOURCE_DIR}/share/seis)

add_library (determine_size SHARED determine_size.cpp swap_bytes.cpp)
add_library (do_load SHARED do_load.cpp swap_bytes.cpp)
add_library (do_write SHARED do_write.cpp swap_bytes.cpp)

install (FILES do_load.hpp determine_size.hpp do_write.hpp
  DESTINATION include/seis
)

install (TARGETS determine_size do_load do_write
  LIBRARY DESTINATION libexec/sdpa
)
