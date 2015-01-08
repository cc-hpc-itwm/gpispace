
include_directories (src)

add_library(determine_size SHARED
            ./src/determine_size.cpp
            ./src/swap_bytes.cpp
           )

add_library(do_load SHARED
            ./src/do_load.cpp
            ./src/swap_bytes.cpp
           )

add_library(do_write SHARED
            ./src/do_write.cpp
            ./src/swap_bytes.cpp
           )

install(FILES src/do_load.hpp src/determine_size.hpp src/do_write.hpp
  DESTINATION include/seis
  )

install(TARGETS determine_size do_load do_write
  LIBRARY
  DESTINATION libexec/sdpa
)
