add_subdirectory(fake)

install(FILES
  pc.hpp
  DESTINATION include/fvm-pc
  COMPONENT runtime-headers
)
