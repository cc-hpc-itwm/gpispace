set(hdrs
  exception.hpp
  plugin.hpp
  kernel.hpp
  plugin_kernel_mediator.hpp
  file_storage.hpp
  null_storage.hpp
)

set(srcs
  plugin.cpp
  kernel.cpp
  plugin_kernel_mediator.cpp
  file_storage.cpp
  null_storage.cpp
)

add_library(fhg-plugin-core ${srcs})
add_library(fhg-plugin-core-shared SHARED ${srcs})
target_link_libraries(fhg-plugin-core-shared ${Boost_FILESYSTEM_SHARED_LIBRARY})
set_target_properties(fhg-plugin-core-shared PROPERTIES OUTPUT_NAME fhg-plugin-core)

install (TARGETS fhg-plugin-core
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries
)

install (TARGETS fhg-plugin-core-shared
  LIBRARY
  DESTINATION lib
  COMPONENT libraries
)

install (FILES ${hdrs}
  DESTINATION include/fhg/plugin/core
  COMPONENT headers
)
